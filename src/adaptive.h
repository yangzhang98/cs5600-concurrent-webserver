#ifndef ADAPTIVE_H
#define ADAPTIVE_H

#include "threadpool.h"

/* AdaptivePool monitor thread (research extension).
 * Sprint 4: periodically sample queue length; grow the pool exponentially
 * (1, 2, 4, ... per round, like Apache httpd's idle_spawn_rate) when the
 * queue stays long, shrink by one per round when it stays empty,
 * always within [min_workers, max_workers]. */
void adaptive_start(threadpool_t *p, int min_workers, int max_workers);
void adaptive_stop(void);

#endif
