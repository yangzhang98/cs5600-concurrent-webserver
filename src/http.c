#include "http.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define REQ_BUF 8192

static const char BODY[] = "Hello, World!\n";

/* write() can send fewer bytes than asked, so loop until everything is out. */
static int write_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

/* Read until we see the end of the headers ("\r\n\r\n"), the buffer is full,
 * or the client closes. We don't use the request content yet. */
static ssize_t read_request(int fd, char *buf, size_t cap) {
    size_t used = 0;
    while (used < cap - 1) {
        ssize_t n = read(fd, buf + used, cap - 1 - used);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break;                 /* client closed */
        used += (size_t)n;
        buf[used] = '\0';
        if (strstr(buf, "\r\n\r\n")) break; /* end of headers */
    }
    buf[used] = '\0';
    return (ssize_t)used;
}

void handle_request(int fd) {
    char req[REQ_BUF];
    ssize_t n = read_request(fd, req, sizeof(req));
    if (n <= 0) return;

    /* Log just the request line, e.g. "GET / HTTP/1.1" */
    char *eol = strstr(req, "\r\n");
    if (eol) *eol = '\0';
    printf("[http] %s\n", req);
    fflush(stdout);

    char header[256];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %zu\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        sizeof(BODY) - 1);

    if (write_all(fd, header, (size_t)hlen) < 0) return;
    write_all(fd, BODY, sizeof(BODY) - 1);
}
