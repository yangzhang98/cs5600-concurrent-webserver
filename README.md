# AdaptivePool: Concurrent Web Server with an Adaptive Thread Pool

CS 5600 term project, OSTEP Project G.5 (Concurrent Web Server).

We are building a multi-threaded web server in C, using a master thread, a bounded request buffer and a pool of worker threads (producer/consumer), with FIFO and Smallest-File-First scheduling. Our research extension, **AdaptivePool**, resizes the worker pool at runtime based on queue length. We will evaluate it against fixed-size pools under steady and bursty synthetic traffic.

## Language

**C** (C11 + POSIX threads). We chose C because OSTEP's examples are in C, and the real thread pool code we studied (nginx, Apache httpd) is in C, so we can compare our design to it directly.

## Build & Run

Tested on Ubuntu (gcc) and macOS (clang). Both build with **0 warnings**.

### 1. Install the tools (first time only)

- **Ubuntu / Debian:**
  ```bash
  sudo apt update
  sudo apt install -y build-essential git curl
  ```
- **macOS:** `xcode-select --install` (installs `clang`, `make` and `git`; `curl` is already there).

### 2. Get the code and build

```bash
git clone https://github.com/yangzhang98/cs5600-concurrent-webserver.git
cd cs5600-concurrent-webserver
make                 # builds ./wserver
```

### 3. Start the server (terminal 1)

```bash
./wserver -p 8080 -t 4 -b 16 -s FIFO
```

You should see:

```
wserver listening on port 8080 (threads=4, buffers=16, sched=FIFO)
```

The server keeps running in this terminal and waits for connections. This is expected.

### 4. Send a request (terminal 2)

Open a **second** terminal and run:

```bash
curl -i http://localhost:8080/
```

Expected output:

```
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 14
Connection: close

Hello, World!
```

Terminal 1 also prints the request line, e.g. `[http] GET / HTTP/1.1`. You can also open `http://localhost:8080/` in a browser.

### 5. Stop the server

Go back to terminal 1 and press **Ctrl+C**. The server catches `SIGINT`, closes the listening socket, frees the pool and exits:

```
shutting down...
```

```bash
echo $?        # prints 0 = clean exit
make clean     # removes the .o files and ./wserver
```

### Command-line options

| Flag | Meaning | Default |
|---|---|---|
| `-p` | port | 10000 |
| `-t` | number of worker threads (stub in Sprint 2) | 1 |
| `-b` | request buffer size (stub in Sprint 2) | 1 |
| `-s` | scheduling policy: `FIFO` or `SFF` | FIFO |

An invalid option (e.g. `./wserver -s BAD`) prints the usage line and exits with code 1.

### Troubleshooting

| Problem | Cause / fix |
|---|---|
| `bind: Address already in use` | Another server is still using the port. Stop it, or use another port, e.g. `-p 8081`. |
| `curl: (7) Failed to connect ... Connection refused` | The server isn't running, or the port in `curl` doesn't match `-p`. |
| `make: command not found` / `cc: command not found` | Install the tools in step 1. |

## Contribution Tracker

**[Contribution tracker (Excel on Northeastern OneDrive)](https://northeastern-my.sharepoint.com/:x:/r/personal/zhang_yang18_northeastern_edu/Documents/Microsoft%20Teams%20Chat%20Files/contribution_tracker.xlsx?d=wcce97c96934d48a9a7fdd83f74f0cf0c&csf=1&web=1&e=Hk1CsO)**. Shared with the professor and TAs. A snapshot is also in `docs/contribution_tracker.xlsx`.

## Sprint 2 Status (skeleton)

| File | Role (matches our sequence diagrams) | Status |
|---|---|---|
| `src/main.c` | master thread: parse args, create pool, accept loop, clean shutdown | done |
| `src/net.c` | `socket → bind → listen` | done |
| `src/http.c` | read request headers, send a hardcoded `Hello, World!` | done (hardcoded) |
| `src/threadpool.c` | bounded buffer + workers; struct, mutex and condition variables defined | **stub**: requests are handled on the master thread for now |
| `src/adaptive.c` | AdaptivePool monitor thread | **stub** |

## Use of Open-Source Code

No third-party code is copied into this repository; the skeleton was written from scratch. In Sprint 1 we planned to start from the OSTEP starter code, but we changed that so the codebase stays small and every line is easy to explain. We studied nginx's `ngx_thread_pool.c` and Apache httpd's worker MPM as references (see `docs/bibliography.md`), and our thread pool follows the same "one mutex + condition variable + queue" pattern.

## Deliverables

### Sprint 2

| Deliverable | Location |
|---|---|
| Annotated bibliography + Research Extension Update | [`docs/bibliography.md`](docs/bibliography.md) |
| Sequence diagram review (what changed and why) | [`docs/architecture/sprint2_diagram_notes.md`](docs/architecture/sprint2_diagram_notes.md) |
| AI reflection log | [`docs/ai-reflections.md`](docs/ai-reflections.md) |
| Skeleton code | `src/`, `Makefile` |

### Sprint 1

| Deliverable | Location |
|---|---|
| Team bio | [`docs/team_bio.pdf`](docs/team_bio.pdf) |
| Executive summary & research proposal | [`docs/proposal.pdf`](docs/proposal.pdf) |
| Exploration report (3 existing systems) | [`docs/exploration.md`](docs/exploration.md), screenshots in `docs/images/` |
| System architecture (3 UML sequence diagrams) | [`docs/architecture/sequence_diagrams.pdf`](docs/architecture/sequence_diagrams.pdf), PNGs + Mermaid sources in `docs/architecture/` (extension sketches in `docs/architecture/extension_cases/`) |

## Repository Layout

```
README.md
Makefile
src/                      our web server (C)
  main.c  net.c  http.c  threadpool.c  adaptive.c  (+ headers)
docs/
  bibliography.md         Sprint 2 annotated bibliography
  ai-reflections.md       AI tool usage, one entry per member per sprint
  team_bio.pdf
  proposal.pdf
  exploration.md
  images/                 screenshots from our VM
  architecture/           sequence diagrams (.mmd, .png) + Sprint 1 PDF + Sprint 2 notes
    extension_cases/      diagrams for the research extension
  src/                    Markdown sources for the PDFs
  contribution_tracker.xlsx
exploration/              test apps + load script used in the exploration report
  flask_app/  express_app/  springboot_app/  run_load.sh
```

## Team

- Yang (Selena) Zhang (@yangzhang98)
- Minyuan Wang (@minyuan1209)
- Yao Qin (@Aureliasusu)
