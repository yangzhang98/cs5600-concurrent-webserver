# Preliminary Exploration: Existing Web Server Implementations

<!-- TODO before submitting: fill in every [TODO] below with your own VM results and replace each screenshot placeholder. -->

## Goal

Before designing our own concurrent web server, we ran three widely used web server stacks on our VM under the same workload and looked at **how each one handles concurrency**: where requests wait, how many threads or processes do the work, and what happens when requests are CPU-heavy versus I/O-heavy. Each stack uses a different concurrency model, so together they cover the main design options we could take.

| System | Language | Concurrency model |
|---|---|---|
| Flask + gunicorn | Python | Pre-forked worker **processes**, one request at a time per process (sync workers) |
| Express (Node.js) | JavaScript | **Single-threaded event loop** with non-blocking I/O |
| Spring Boot (embedded Tomcat) | Java | Fixed-max **thread pool**, one thread per request, plus a separate accept queue |

## Test Setup

- **VM:** [TODO: OS, number of vCPUs, RAM]
- **Load tool:** `wrk` (`sudo apt install wrk`), 4 client threads, 20 seconds per run, `--latency` for percentiles. All runs use `exploration/run_load.sh`, so every system gets exactly the same test.
- **Test apps:** each system serves the same three endpoints (code in `exploration/`):
  - `/hello`: tiny response, measures raw request-handling overhead
  - `/io?ms=20`: waits 20 ms without using the CPU, like a database or disk call
  - `/cpu?ms=20`: busy-loops for 20 ms, like a compute-heavy request
- **Concurrency levels:** 8 and 64 open connections.

How to reproduce:

```bash
# Flask + gunicorn (4 worker processes)
cd exploration/flask_app && pip install -r requirements.txt
gunicorn -w 4 -b 0.0.0.0:8001 app:app
# Express
cd exploration/express_app && npm install && node server.js
# Spring Boot (needs JDK 17 and Maven)
cd exploration/springboot_app && mvn -q spring-boot:run

# In another terminal, for each server:
cd exploration && ./run_load.sh 8001 flask     # 8002 express, 8003 springboot
```

---

## 1. Flask + gunicorn

**How it works.** Flask is just the application framework, and its built-in server is only meant for development. In production it runs behind gunicorn. With the default `sync` worker type, gunicorn starts a master process and forks a fixed number of worker processes (`-w 4`). Every worker accepts connections from the same listening socket and handles **one request at a time**, start to finish. So the maximum number of requests in progress at once equals the number of workers. Extra connections wait in the kernel's accept backlog, not in a queue that gunicorn controls.

**Screenshot:**

![Flask + gunicorn under load](images/flask_load.png)
<!-- TODO: screenshot showing gunicorn running AND the wrk output in the same shot -->

**Results** [TODO: copy from `exploration/results/flask/`]

| Endpoint | Connections | Requests/sec | p50 | p99 |
|---|---|---|---|---|
| /hello | 64 | | | |
| /io?ms=20 | 8 | | | |
| /io?ms=20 | 64 | | | |
| /cpu?ms=20 | 64 | | | |

**Analysis.** [TODO: rewrite in your own words once you have numbers. Things to check:]
- For `/io?ms=20` with 4 workers, throughput should top out around 4 / 0.020 s ≈ 200 req/s no matter how many connections you add, because a worker that is sleeping cannot take another request. Does your data match this?
- What happens to p99 when you go from 8 to 64 connections? The extra connections just wait in the backlog, so the waiting time should show up in tail latency.
- Strength: processes are isolated from each other, and Python's GIL does not limit throughput across processes.
- Weakness: the pool size is fixed when the server starts. If it is too small, I/O-heavy traffic wastes the CPU while workers sleep. If it is too large, each process costs a lot of memory.

## 2. Express (Node.js)

**How it works.** Node runs all JavaScript on **one event-loop thread**. Network I/O is non-blocking, and timers and sockets are handled by libuv. Libuv also has a small internal thread pool (4 threads by default), but it is only used for things like file system calls and DNS, not for running request handlers. A request handler that waits on I/O (`setTimeout`, an async DB call) gives the event loop back so other requests can run. A handler that does CPU work holds the only thread until it finishes.

**Screenshot:**

![Express under load](images/express_load.png)
<!-- TODO: screenshot showing node running AND the wrk output -->

**Results** [TODO: copy from `exploration/results/express/`]

| Endpoint | Connections | Requests/sec | p50 | p99 |
|---|---|---|---|---|
| /hello | 64 | | | |
| /io?ms=20 | 8 | | | |
| /io?ms=20 | 64 | | | |
| /cpu?ms=20 | 64 | | | |

**Analysis.** [TODO: rewrite in your own words once you have numbers. Things to check:]
- `/io` should scale well with more connections, since thousands of waits can overlap on one thread with no extra threads needed.
- `/cpu` should be close to the worst case: about 1 / 0.020 s ≈ 50 req/s, and every request queues behind the one that is currently computing, so p99 should grow roughly with the number of connections.
- This is the clearest example of a slow request hurting everyone else's latency. It is exactly what our multi-threaded design should avoid for CPU-heavy requests.

## 3. Spring Boot (embedded Tomcat)

**How it works.** Spring Boot runs an embedded Tomcat server. Tomcat has an acceptor thread that takes new connections and a **worker thread pool** (`server.tomcat.threads.max`, default 200) that runs the request handlers, one thread per request for the whole request. When all worker threads are busy, new connections wait in a queue whose size is set by `accept-count` (default 100). This is the closest of the three to our own design: one thread accepting connections and a bounded pool of workers behind it.

**Screenshot:**

![Spring Boot under load](images/springboot_load.png)
<!-- TODO: screenshot showing the Spring Boot log AND the wrk output -->

**Results** [TODO: copy from `exploration/results/springboot/`]

| Endpoint | Connections | Requests/sec | p50 | p99 |
|---|---|---|---|---|
| /hello | 64 | | | |
| /io?ms=20 | 8 | | | |
| /io?ms=20 | 64 | | | |
| /cpu?ms=20 | 64 | | | |

**Analysis.** [TODO: rewrite in your own words once you have numbers. Things to check:]
- With 200 threads, `/io` at 64 connections should barely queue at all, because each blocked request has its own thread.
- `/cpu` is limited by the number of vCPUs, not by the thread count. 200 threads on a few cores just take turns, so latency goes up because of context switching instead of queueing.
- Optional extra experiment: set `server.tomcat.threads.max=4` and rerun `/io` at 64 connections. The thread pool size now becomes the bottleneck, which shows how sensitive a fixed pool is to its size.
- The default of 200 threads is a "just make it big" choice. It works for I/O-heavy traffic but uses a lot of memory and does not adapt to the actual load.

---

## Comparison

| | Flask + gunicorn | Express | Spring Boot |
|---|---|---|---|
| Unit of concurrency | Process | Callback on one thread | Thread |
| Where waiting requests sit | Kernel accept backlog | Event loop queue | Tomcat accept queue |
| Good at | Isolation | Lots of I/O waiting | Mixed workloads |
| Weak at | Fixed small pool, I/O-heavy traffic | CPU-heavy requests | Memory, fixed large pool |
| Pool size adapts to load? | No | N/A | No (fixed max) |
| /io p99 at 64 connections | [TODO] | [TODO] | [TODO] |
| /cpu p99 at 64 connections | [TODO] | [TODO] | [TODO] |

## What This Means for Our Design

1. **Separating "accept" from "work" is the right structure.** Spring Boot / Tomcat uses a dedicated acceptor and a worker pool, which is the same master/worker producer-consumer design the OSTEP project asks for. Unlike gunicorn, it lets us see and control the queue ourselves, so we can measure queueing delay directly.
2. **No single fixed pool size works for every workload.** Gunicorn's small fixed pool limits I/O-heavy traffic, while Tomcat's large fixed pool wastes resources and does nothing extra for CPU-heavy traffic. This is the gap our research extension (AdaptivePool) is aimed at: sizing the pool from the observed queue length instead of picking a number ahead of time.
3. **CPU-heavy requests need real threads.** Express shows that an event loop alone cannot stop one CPU-heavy request from delaying everyone else. Our workload generator should include CPU-bound and I/O-bound requests separately so we can see this effect in our own server.
4. **Tail latency shows the problems that averages hide.** [TODO: point to one specific number from your results where p99 was much worse than p50.]
