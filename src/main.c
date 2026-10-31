#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "adaptive.h"
#include "net.h"
#include "threadpool.h"

static volatile sig_atomic_t running = 1;

static void on_sigint(int sig) {
    (void)sig;
    running = 0;
}

static void usage(const char *prog) {
    fprintf(stderr, "usage: %s [-p port] [-t threads] [-b buffers] [-s FIFO|SFF]\n", prog);
    exit(1);
}

int main(int argc, char *argv[]) {
    int port = 10000, threads = 1, buffers = 1;
    sched_policy_t policy = SCHED_FIFO_POLICY;

    int c;
    while ((c = getopt(argc, argv, "p:t:b:s:")) != -1) {
        switch (c) {
        case 'p': port = atoi(optarg); break;
        case 't': threads = atoi(optarg); break;
        case 'b': buffers = atoi(optarg); break;
        case 's':
            if (strcmp(optarg, "FIFO") == 0)     policy = SCHED_FIFO_POLICY;
            else if (strcmp(optarg, "SFF") == 0) policy = SCHED_SFF_POLICY;
            else usage(argv[0]);
            break;
        default: usage(argv[0]);
        }
    }
    if (port <= 0 || port > 65535 || threads <= 0 || buffers <= 0) usage(argv[0]);

    /* Ctrl+C -> running = 0. No SA_RESTART, so a blocked accept() returns EINTR
     * and the loop below can exit cleanly. */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* A client that disconnects mid-write would otherwise kill us with SIGPIPE. */
    signal(SIGPIPE, SIG_IGN);

    threadpool_t *pool = pool_create(threads, buffers, policy);
    if (!pool) {
        fprintf(stderr, "pool_create failed\n");
        return 1;
    }
    adaptive_start(pool, 1, threads);   /* no-op for now */

    int listen_fd = open_listen_fd(port);
    if (listen_fd < 0) {
        pool_destroy(pool);
        return 1;
    }
    printf("wserver listening on port %d (threads=%d, buffers=%d, sched=%s)\n",
           port, threads, buffers, policy == SCHED_FIFO_POLICY ? "FIFO" : "SFF");
    fflush(stdout);

    while (running) {
        int conn_fd = accept(listen_fd, NULL, NULL);
        if (conn_fd < 0) {
            if (errno == EINTR) continue;   /* woken by signal; re-check running */
            perror("accept");
            continue;
        }
        pool_submit(pool, conn_fd);
    }

    printf("\nshutting down...\n");
    close(listen_fd);
    adaptive_stop();
    pool_destroy(pool);
    return 0;
}
