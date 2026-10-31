# Sprint 2: Sequence Diagram Review

## Main diagrams (01–03): no changes needed

After reading real thread pool code, our three main sequence diagrams still match how production servers do it, so we didn't redraw them.

| Our diagram | What we compared it to | Result |
|---|---|---|
| **01 Normal request (FIFO)**: master locks, puts the request in the buffer, signals `not_empty`, unlocks | nginx `ngx_thread_task_post()` ([src/core/ngx_thread_pool.c, L231](https://github.com/nginx/nginx/blob/release-1.31.8/src/core/ngx_thread_pool.c#L231)): lock mutex → signal cond → append task → unlock | Same steps. |
| **02 Worker wait and wake-up**: workers call `wait(not_empty)` inside a `while (count == 0)` loop | nginx `ngx_thread_pool_cycle()` ([L278](https://github.com/nginx/nginx/blob/release-1.31.8/src/core/ngx_thread_pool.c#L278)): `for(;;)` loop with `while (tp->queue.first == NULL) cond_wait(...)` | Same pattern. The `while` (not `if`) matters because of spurious wakeups, and our diagram already had it. |
| **03 Buffer full / backpressure**: master waits on `not_full` | nginx does *not* block here: if `waiting >= max_queue` it logs "queue overflow" and returns an error | Different on purpose. We keep blocking backpressure (no dropped requests, matches OSTEP's bounded buffer). The reasoning is in the Research Extension Update in `docs/bibliography.md`. |

So the control flow we drew in Sprint 1 is still correct; the reading confirmed it.

The Sprint 2 skeleton code is organized around these diagrams too: `main.c` = master thread, `threadpool.c` = buffer + workers, `http.c` = request handling.

## Extension diagram ext_B (AdaptivePool resizing): updated

File: `extension_cases/ext_B_adaptive_pool_resize.mmd` / `.png`

Only the grow branch changed:

- **Before:** queue long for k samples → `n_workers++`, create **one** new worker.
- **After:** queue long for k samples → create `add = min(spawn_rate, max - n_workers)` workers, then double `spawn_rate` (capped at `SPAWN_CAP`). So a burst gets 1, 2, 4, … new threads per round instead of one at a time. `spawn_rate` resets to 1 in the other two branches.
- **Shrink branch:** unchanged, still retires one idle worker per round.

This follows Apache httpd's `perform_idle_server_maintenance()` ([server/mpm/worker/worker.c, L1400–L1605](https://github.com/apache/httpd/blob/2.4.69/server/mpm/worker/worker.c#L1400-L1605), httpd 2.4.69), which doubles `idle_spawn_rate` each round while there are too few idle threads and kills off only one child per round when there are too many. See the Research Extension Update for details.

Note: `sequence_diagrams.pdf` is our Sprint 1 submission and still shows the old ext_B. The `.mmd`/`.png` files above are the current version.
