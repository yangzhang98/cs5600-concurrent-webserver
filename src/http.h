#ifndef HTTP_H
#define HTTP_H

/* Read the request headers from fd and send back a hardcoded
 * "200 OK / Hello, World!" response. Does NOT close fd (caller does).
 * Sprint 3: parse the request line and serve a real file. */
void handle_request(int fd);

#endif
