#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

typedef enum { SCHED_FIFO_POLICY, SCHED_SFF_POLICY } sched_policy_t;

typedef struct {
    /* Bounded queue (circular buffer) of connection fds */
    int *buf;
    int  cap, head, tail, count;

    /* One lock + two condition variables (sequence diagrams 1-3) */
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;   /* workers wait here when queue is empty */
    pthread_cond_t  not_full;    /* main thread waits here when queue is full */

    pthread_t *workers;
    int  n_workers;
    sched_policy_t policy;
} threadpool_t;

threadpool_t *pool_create(int n_workers, int queue_cap, sched_policy_t policy);
void          pool_submit(threadpool_t *p, int fd);   /* producer side */
void          pool_destroy(threadpool_t *p);

#endif
