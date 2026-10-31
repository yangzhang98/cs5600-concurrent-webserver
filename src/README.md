Our concurrent web server in C. See the root `README.md` for build/run instructions.

| File | Role |
|---|---|
| `main.c` | master thread: args, accept loop, SIGINT shutdown |
| `net.c/.h` | listening socket |
| `http.c/.h` | request handling (hardcoded response in Sprint 2) |
| `threadpool.c/.h` | bounded buffer + worker threads (stub in Sprint 2) |
| `adaptive.c/.h` | AdaptivePool monitor thread (stub until the research extension) |
