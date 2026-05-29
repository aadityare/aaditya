#include "ssh.h"
#include "../config.h"
#include "../net/listener.h"
#include "../util/io.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/socket.h>

static int wait_readable(int fd, int timeout_ms) {
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    return poll(&pfd, 1, timeout_ms);
}

static void proxy_fds(int a, int b) {
    char buf[BUF];
    fd_set fds;
    int maxfd = (a > b ? a : b) + 1;
    while (1) {
        FD_ZERO(&fds);
        FD_SET(a, &fds);
        FD_SET(b, &fds);
        if (select(maxfd, &fds, NULL, NULL, NULL) <= 0) break;
        if (FD_ISSET(a, &fds)) {
            ssize_t n = read(a, buf, sizeof(buf));
            if (n <= 0) break;
            if (write_all(b, buf, (size_t)n) < 0) break;
        }
        if (FD_ISSET(b, &fds)) {
            ssize_t n = read(b, buf, sizeof(buf));
            if (n <= 0) break;
            if (write_all(a, buf, (size_t)n) < 0) break;
        }
    }
}

void *handle_ssh(void *arg) {
    SSHArg *a = arg;
    int cfd = a->fd;
    free(a);
    pthread_detach(pthread_self());

    if (wait_readable(cfd, 10000) <= 0) {
        close(cfd);
        return NULL;
    }

    char peek[16] = {0};
    ssize_t got = 0;
    for (int attempts = 0; attempts < 5 && got < 4; attempts++) {
        got = recv(cfd, peek, sizeof(peek) - 1, MSG_PEEK);
        if (got < 4 && attempts < 4)
            wait_readable(cfd, 500);
    }

    if (got < 4 || strncmp(peek, "SSH-", 4) != 0) {
        close(cfd);
        return NULL;
    }

    int sfd = connect_internal(SSH_INTERNAL_HOST, SSH_INTERNAL_PORT);
    if (sfd < 0) {
        const char msg[] = "SSH service unavailable.\r\n";
        write_all(cfd, msg, sizeof(msg) - 1);
        close(cfd);
        return NULL;
    }

    proxy_fds(cfd, sfd);
    close(sfd);
    close(cfd);
    return NULL;
}
