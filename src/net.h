#ifndef NET_H
#define NET_H

/* socket -> setsockopt(SO_REUSEADDR) -> bind -> listen.
 * Returns the listening fd, or -1 on error. */
int open_listen_fd(int port);

#endif
