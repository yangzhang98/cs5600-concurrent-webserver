#include "threadpool.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Consumer loop. STUB for Sprint 2 - no worker threads are started yet.
 *
 * Sprint 3 plan (same pattern as nginx ngx_thread_pool_cycle):
 *   lock
 *   while (count == 0) wait(not_empty)     <- while, not if: spurious wakeups
 *   fd = take from queue (FIFO: head; SFF: smallest file)
 *   signal(not_full)
 *   unlock
 *   handle_request(fd); close(fd)
 */
__attribute__((unused))
static void *worker_loop(void *arg) {
    (void)arg;
    return NULL;
}

threadpool_t *pool_create(int n_workers, int queue_cap, sched_policy_t policy) {
    threadpool_t *p = calloc(1, sizeof(*p));
    if (!p) return NULL;

    p->buf = calloc((size_t)queue_cap, sizeof(int));
    p->workers = calloc((size_t)n_workers, sizeof(pthread_t));
    if (!p->buf || !p->workers) {
        free(p->buf);
        free(p->workers);
        free(p);
        return NULL;
    }

    p->cap = queue_cap;
    p->n_workers = n_workers;
    p->policy = policy;
    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->not_empty, NULL);
    pthread_cond_init(&p->not_full, NULL);

    /* Sprint 3: for i in 0..n_workers: pthread_create(&p->workers[i], NULL, worker_loop, p); */
    return p;
}

void pool_submit(threadpool_t *p, int fd) {
    (void)p;
    /* Sprint 2: handle the request right here on the main thread.
     *
     * Sprint 3 (producer side, like nginx ngx_thread_task_post):
     *   lock
     *   while (count == cap) wait(not_full)    <- backpressure, diagram 3
     *   put fd at tail
     *   signal(not_empty)
     *   unlock
     */
    handle_request(fd);
    close(fd);
}

void pool_destroy(threadpool_t *p) {
    if (!p) return;
    /* Sprint 3: set a shutdown flag, broadcast not_empty, join all workers. */
    pthread_mutex_destroy(&p->lock);
    pthread_cond_destroy(&p->not_empty);
    pthread_cond_destroy(&p->not_full);
    free(p->buf);
    free(p->workers);
    free(p);
}
