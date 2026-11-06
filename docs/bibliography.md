# Sprint 2: Annotated Bibliography

Project G.5, Concurrent Web Server · Research extension: **AdaptivePool**

Numbers in square brackets, e.g. [3], refer to the entries below.

## Research Extension Update

**Short version:** our direction (AdaptivePool) did not change, but the reading changed how we frame it, and we made three concrete changes to the design and the evaluation plan.

### What the reading told us about our direction

- **The idea is not new, and that is useful to know.** SEDA [2] already had a *thread pool controller* (§3.4) that samples a stage's input queue, adds a thread when the queue length passes a threshold (up to a per-stage maximum), and removes threads that have been idle for a while. That is very close to AdaptivePool. Our Sprint 1 proposal already cited SEDA as the origin of the idea; the reading made it clear that our contribution is *re-evaluating* this policy in a small thread-pool server under the bursty workloads we care about, not inventing it.
- **Why most web servers today don't do this (the "why does the gap exist?" question from our Sprint 1 feedback).**
  1. **The mainstream moved to event-driven designs.** Flash [1] showed that a single-process event-driven server with helpers for blocking disk I/O can match or beat multi-process and multi-threaded servers, and nginx's workers are single-threaded event loops [5]. In that model threads are not the unit of concurrency, so "how many threads?" mostly stops being a question.
  2. **Where production servers do use thread pools, the pool is for blocking work and is fixed in size.** nginx uses a pool only to offload blocking file I/O, created once at startup with 32 threads by default [6][9]. Cloudflare got a 6x p99 improvement from moving more blocking calls into that pool, not from resizing it [12].
  3. **More threads don't add CPU.** Arachne [3] argues that the resource to manage is *cores*, not threads. McClure et al. [11] found that for very short tasks, *static* core allocations often beat reallocating cores at runtime, because reallocation has its own cost.
  4. **The one major server that does resize, Apache httpd, resizes processes and does it cautiously** [10], which shows how much care the control policy needs.
- **nginx is the "fixed pool" baseline we're testing against.** Its default pool has 32 threads and never changes size at runtime [9], which is exactly the design our experiment questions.

### What we changed

1. **Grow exponentially, shrink by one.** httpd's `perform_idle_server_maintenance()` [10] doubles `idle_spawn_rate` each maintenance round while there are too few idle threads (1 → 2 → 4 …, capped by `MAX_SPAWN_RATE` = 32), resets it to 1 once things are fine, and kills off only **one** child per round when there are too many idle threads. httpd scales *child processes* (each holding `ThreadsPerChild` threads), while we scale threads directly, but we are adopting the same policy: double the number of threads added per round while the queue stays long, remove one thread per round while it stays empty. Appendix diagram `ext_B` was updated to match (see `docs/architecture/sprint2_diagram_notes.md`).
2. **Measure per-worker load, not just p99.** Majkowski [7] shows that having more workers does not mean work is spread evenly: with epoll's wakeup order, a few nginx workers end up doing most of the work. So besides p99 latency we will log **how many requests each worker handled**. If AdaptivePool adds threads that mostly sit idle, the extra threads aren't really helping.
3. **Keep blocking backpressure, but document the trade-off.** nginx's `ngx_thread_task_post()` [9] does *not* block when its queue is full; it logs "queue overflow" and returns an error. Our design (sequence diagram 03) makes the master thread wait on `not_full`, so no request is dropped, which matches OSTEP's bounded buffer. We'll note in the final paper that nginx makes the opposite choice (fail fast instead of slowing down `accept()`).

### Where we expect AdaptivePool to lose (added to our experiment plan)

Our Sprint 1 feedback asked us to look for the cases where our system could do worse than the baseline. Based on the reading, we will run these on purpose:

- **CPU-bound requests on few cores.** Extra threads add context switches without adding CPU [3]. This is the same effect behind the wall-clock trap we found in our Sprint 1 exploration.
- **Bursts shorter than the sampling interval.** The controller reacts after the burst is over and only adds overhead.
- **Load that rises and falls with a period close to our shrink window.** The pool could keep growing and shrinking (flapping), which is why httpd [10] shrinks one step at a time.
- **Pool already at its maximum.** Only the monitor's cost remains.
- **Steady traffic.** There is nothing to adapt to, so a well-sized fixed pool should match or beat us, consistent with McClure et al.'s finding that static allocation often wins [11].

### Hypothesis (updated wording)

Under bursty traffic, AdaptivePool (exponential grow, one-at-a-time shrink, bounded by `[min, max]` threads) will get lower p99 latency than fixed pools of 1/2/4/8 threads while keeping a similar or lower average thread count, and per-worker request counts will show that the added threads actually do work. Under steady or CPU-bound traffic on few cores, we expect it to be no better than, and possibly slightly worse than, the best fixed pool.

---

## Annotated Bibliography

### [1] Vivek S. Pai, Peter Druschel, Willy Zwaenepoel, "Flash: An Efficient and Portable Web Server," Proceedings of the 1999 USENIX Annual Technical Conference, pp. 199–212, 1999. [Link](https://static.usenix.org/event/usenix99/full_papers/pai/pai.pdf)

**Category:** Academic Paper (classic)

**Summary:** The paper introduces the asymmetric multi-process event-driven (AMPED) architecture: a single event-driven process serves cached content, and helper processes handle blocking disk operations so the main loop never stalls. The authors build multi-process (MP), multi-threaded (MT), single-process event-driven (SPED), and AMPED versions from one code base, so the measured differences come from the concurrency architecture itself.

**Relevance:** Our server is in the MT family, and this paper is the clearest comparison of why one would pick threads over an event loop. It also explains why many modern servers never face our "how many threads?" question: in an event-driven design, threads are only helpers for blocking work.

### [2] Matt Welsh, David Culler, Eric Brewer, "SEDA: An Architecture for Well-Conditioned, Scalable Internet Services," Proceedings of the 18th ACM Symposium on Operating Systems Principles (SOSP '01), pp. 230–243, 2001. [Link](https://www.cs.princeton.edu/courses/archive/fall15/cos518/papers/seda.pdf)

**Category:** Academic Paper (classic)

**Summary:** SEDA splits a service into stages connected by event queues, each stage with its own thread pool, so the server stays "well-conditioned" (degrades gracefully) under overload. Section 3.4 describes dynamic resource controllers, including a thread pool controller that adds threads when a stage's queue passes a threshold and removes threads that have been idle for a while.

**Relevance:** SEDA's thread pool controller is the direct ancestor of AdaptivePool: same signal (queue length), same actions (add when long, remove when idle). It lets us frame our project as re-testing a known policy in a simple, single-queue thread-pool server instead of claiming a new idea.

### [3] Henry Qin, Qian Li, Jacqueline Speiser, Peter Kraft, John Ousterhout, "Arachne: Core-Aware Thread Management," 13th USENIX Symposium on Operating Systems Design and Implementation (OSDI '18), pp. 145–160, 2018. [Link](https://www.usenix.org/system/files/osdi18-qin.pdf)

**Category:** Academic Paper (contemporary)

**Summary:** Arachne is a user-level threading system where each application decides how many cores it needs based on its load, and a central core arbiter hands cores out between applications. Adding it to memcached cut tail latency by more than 10x and improved SLO-compliant throughput by 37%.

**Relevance:** Arachne adapts to load like we do, but it adapts *cores*, not *threads*, and that distinction is our main threat: on a VM with few cores, adding threads cannot add CPU. This shaped our "where we expect to lose" list, especially CPU-bound requests on few cores.

### [4] Derek DeJonghe, "Chapter 30: Performance Tuning," *Complete NGINX Cookbook: Advanced Recipes for Operations*, O'Reilly Media, 2017. [Link](https://ftp.jaist.ac.jp/pub/sourceforge.jp/c3rb3ru5prjct/69609/Complete_NGINX_Cookbook.pdf)

**Category:** Book

**Summary:** This chapter is a set of practical tuning recipes for nginx: start by automating tests with a load driver to set a baseline and measure every change (§30.1), keep connections open to clients and upstream servers (§30.2–30.3), buffer responses and access logs (§30.4–30.5), and tune the OS (§30.6). The OS section covers `net.core.somaxconn`, the kernel queue of connections waiting for nginx to accept them, and raising file descriptor limits for many connections.

**Relevance:** Its "baseline first, then change one thing and re-measure" method is exactly how our Sprint 3 benchmark rig and Sprint 4 comparison are set up, with fixed pools as the baseline. Its point about `somaxconn` is the kernel side of our sequence diagram 03: when our buffer is full and the master stops calling `accept()`, new clients wait in that kernel queue (our `listen()` backlog of 128).

### [5] Andrew Alexeev, "nginx," in Amy Brown and Greg Wilson (eds.), *The Architecture of Open Source Applications, Volume II*, Chapter 14, Lulu.com, 2012. [Link](https://aosabook.org/en/v2/nginx.html)

**Category:** Book

**Summary:** This chapter explains why nginx (originally written by Igor Sysoev) uses a single master process plus several single-threaded worker processes, each running a non-blocking, event-driven loop over many connections. It argues that this keeps memory and CPU usage manageable as load increases, and saves CPU because there is no ongoing create/destroy cycle of processes or threads per connection.

**Relevance:** It is the long-form reason the most widely used web server doesn't have our pool-sizing problem in its core design. This is the first part of our answer to "why hasn't this been adopted?", and it is the contrast we will use when discussing where a thread-pool server like ours still makes sense.

### [6] Valentin Bartenev, "Thread Pools in NGINX Boost Performance 9x!," F5 / NGINX Blog, June 19, 2015. [Link](https://www.f5.com/company/blog/nginx/thread-pools-boost-performance-9x)

**Category:** Blog

**Summary:** The post explains that a blocking operation, such as reading an uncached file from a slow disk, stalls an nginx worker's whole event loop, and introduces thread pools (a task queue plus threads) to offload that work. In a disk-bound synthetic test, throughput went from about 1 Gbps to 9.5 Gbps, but the author warns that when the data fits in the page cache, the pool only adds overhead.

**Relevance:** It describes the fixed pool we are comparing against (default pool of 32 threads, `max_queue` default 65536, overflow is rejected and logged), written by one of the authors of the code in [9]. Its warning that extra threads only help when work actually blocks is an important input to our CPU-bound vs I/O-bound experiments.

### [7] Marek Majkowski, "Why does one NGINX worker take all the load?," The Cloudflare Blog, October 23, 2017. [Link](https://blog.cloudflare.com/the-sad-state-of-linux-socket-balancing)

**Category:** Blog

**Summary:** The post shows that with a shared listen socket and epoll, Linux tends to wake the most recently waiting process (LIFO-like), so the busiest nginx worker keeps getting new connections. Using `SO_REUSEPORT` (one accept queue per worker) spreads the load more evenly, but in their test worst-case latency roughly doubled (about 144 ms vs 72 ms).

**Relevance:** Our pool also has many workers waiting on one shared queue, and `pthread_cond_signal()` does not promise which waiter wakes up, so "more threads" might not mean "more threads doing work." Because of this post, we added per-worker request counts to our metrics.

### [8] Jonathan M Davis (question) and answerers, "Why does pthread_cond_wait have spurious wakeups?," Stack Overflow, 2011. [Link](https://stackoverflow.com/questions/8594591/why-does-pthread-cond-wait-have-spurious-wakeups)

**Category:** Community

**Summary:** The question asks why POSIX lets `pthread_cond_wait()` return without a signal, which looks like a bug. The answers explain that callers must re-check the condition anyway, since another thread may take the work first, and quote David Butenhof that making wakeups fully predictable could slow down condition variables on some multiprocessor systems, so the spec chose speed and forced robust predicate loops.

**Relevance:** This is the design reason behind the `while (count == 0) wait(not_empty)` loop in sequence diagram 02 and in our Sprint 3 worker code. It matters even more for AdaptivePool, because a worker that wakes up has to decide between "take a request" and "retire," and must re-check both conditions under the lock.

### [9] Nginx, Inc., Valentin V. Bartenev, Ruslan Ermilov, "ngx_thread_task_post() and ngx_thread_pool_cycle()," nginx source code, release-1.31.8, src/core/ngx_thread_pool.c (lines 230–361), 2026. [Link](https://github.com/nginx/nginx/blob/release-1.31.8/src/core/ngx_thread_pool.c#L230-L361)

**Category:** Source Code

**Summary:** `ngx_thread_task_post()` is the producer: it locks the pool mutex, rejects the task with a "queue overflow" error if `waiting >= max_queue`, otherwise signals the condition variable, appends the task, and unlocks. `ngx_thread_pool_cycle()` is the consumer loop each pool thread runs: lock, `while (queue is empty) cond_wait`, take the first task, unlock, run it, then hand the result back to the event loop.

**Relevance:** This is the same producer/consumer structure as our bounded buffer (sequence diagrams 01 and 02), which is why we kept those diagrams unchanged. It also shows a fixed-size design choice: the pool is created at startup with a configured thread count (32 by default) and never resized.

**What we learned:** A production server uses exactly the "one lock + condition variable + queue" pattern from OSTEP, including the `while` loop around the wait. The biggest difference from our design is backpressure: nginx fails fast when the queue is full, while we block the master thread so no request is dropped.

### [10] Apache Software Foundation, "perform_idle_server_maintenance()," Apache httpd source code, 2.4.69, server/mpm/worker/worker.c (lines 1400–1605), 2026. [Link](https://github.com/apache/httpd/blob/2.4.69/server/mpm/worker/worker.c#L1400-L1605)

**Category:** Source Code

**Summary:** This function runs once per maintenance cycle in the parent process. It counts idle worker threads across children; if there are more than `MaxSpareThreads`, it gracefully kills off one child. If there are fewer than `MinSpareThreads`, it spawns `idle_spawn_rate` new children and doubles that rate for the next round, up to `MAX_SPAWN_RATE` (32), resetting the rate to 1 once no spawning is needed.

**Relevance:** It is a real, long-running implementation of "grow fast, shrink slow," the exact policy AdaptivePool is built on. We adopted its exponential spawn rate and one-at-a-time shrink for our updated `ext_B` diagram.

**What we learned:** The unit matters. httpd resizes *processes* (each holding `ThreadsPerChild` threads), not individual threads, and it has extra guards (a hold-off after graceful restarts, limits on total children). These are reminders that a resizing policy needs to protect itself from overreacting.

### [11] Sarah McClure, Amy Ousterhout, Scott Shenker, Sylvia Ratnasamy, "Efficient Scheduling Policies for Microsecond-Scale Tasks," 19th USENIX Symposium on Networked Systems Design and Implementation (NSDI '22), pp. 1–18, 2022. [Link](https://www.usenix.org/system/files/nsdi22-paper-mcclure_2.pdf)

**Category:** Academic Paper (contemporary)

**Summary:** Using simulations, the authors compare policies for allocating cores across applications and for load-balancing tasks across cores, aiming for the best mix of latency and efficiency. They conclude that work stealing is the best load-balancing policy, several core-allocation policies work well, and, surprisingly, static core allocations often outperform reallocation when tasks are small.

**Relevance:** This is the strongest evidence that adapting at runtime can lose to a well-chosen static allocation, which is exactly the "where could we be worse than baseline" question from our feedback. Their tasks are microsecond-scale and ours are milliseconds, so we will test whether their finding still holds at our scale (our steady-traffic and short-burst cases).

### [12] Ka-Hing Cheung, "How we scaled nginx and saved the world 54 years every day," The Cloudflare Blog, July 31, 2018. [Link](https://blog.cloudflare.com/how-we-scaled-nginx-and-saved-the-world-54-years-every-day/)

**Category:** Blog

**Summary:** Cloudflare found that blocking disk operations inside nginx's event loop caused more than half of their p99 time-to-first-byte. Turning on `SO_REUSEPORT` helped peak p99 by 33%, but nginx's built-in thread-pool reads alone did not help them; they had to move most of `open()` into the thread pool too, which improved peak p99 TTFB by about 6x.

**Relevance:** It shows, at production scale, that tail latency comes from work stuck in the wrong place, and that the fix was *what* goes to the pool, not *how big* the pool is. This keeps us honest about p99: AdaptivePool only helps if queueing in our buffer is actually what drives the tail, which our queueing-delay metric (`t_dequeue − t_arrive`) is designed to check.
