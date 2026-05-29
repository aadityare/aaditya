#include "acceptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static pthread_mutex_t rate_mutex = PTHREAD_MUTEX_INITIALIZER;

static int check_rate(Acceptor *acc, uint32_t ip) {
    uint32_t h = (ip * 2654435761u) & (RATE_BUCKETS - 1);
    time_t now = time(NULL);

    for (int i = 0; i < RATE_BUCKETS; i++) {
        uint32_t idx = (h + (uint32_t)i) & (RATE_BUCKETS - 1);
        RateBucket *b = &acc->rate_table[idx];

        if (b->ip == 0) {
            b->ip          = ip;
            b->tokens      = RATE_TOKENS - 1;
            b->last_refill = now;
            return 1;
        }
        if (b->ip == ip) {
            long elapsed = (long)(now - b->last_refill);
            if (elapsed >= RATE_REFILL_SEC) {
                b->tokens      = RATE_TOKENS;
                b->last_refill = now;
            }
            if (b->tokens <= 0) return 0;
            b->tokens--;
            return 1;
        }
    }
    return 1;
}

typedef struct {
    void     *(*handler)(void *);
    Acceptor  *acc;
    void      *inner;
} HandlerWrap;

static void *handler_wrapper(void *arg) {
    HandlerWrap *w = arg;
    w->handler(w->inner);
    __sync_fetch_and_sub(&w->acc->active_conns, 1);
    free(w);
    return NULL;
}

void *accept_loop(void *arg) {
    Acceptor *acc = arg;
    pthread_detach(pthread_self());

    while (1) {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        int cfd = accept(acc->listen_fd, (struct sockaddr*)&addr, &addrlen);
        if (cfd < 0) { perror("accept"); continue; }

        if (acc->active_conns >= MAX_CONNS_PER_PORT) {
            close(cfd);
            continue;
        }

        uint32_t ip = 0;
        if (addr.ss_family == AF_INET) {
            ip = ((struct sockaddr_in*)&addr)->sin_addr.s_addr;
        } else if (addr.ss_family == AF_INET6) {
            unsigned char *b = ((struct sockaddr_in6*)&addr)->sin6_addr.s6_addr;
            memcpy(&ip, b + 12, 4);
        }

        pthread_mutex_lock(&rate_mutex);
        int allowed = check_rate(acc, ip);
        pthread_mutex_unlock(&rate_mutex);

        if (!allowed) { close(cfd); continue; }

        struct timeval tv = { .tv_sec = 30, .tv_usec = 0 };
        setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(cfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        HandlerWrap *w = malloc(sizeof(HandlerWrap));
        if (!w) { close(cfd); continue; }

        int *inner = malloc(acc->arg_size);
        if (!inner) { free(w); close(cfd); continue; }
        *inner = cfd;

        w->handler = acc->handler;
        w->acc     = acc;
        w->inner   = inner;

        __sync_fetch_and_add(&acc->active_conns, 1);

        pthread_t tid;
        if (pthread_create(&tid, NULL, handler_wrapper, w) != 0) {
            __sync_fetch_and_sub(&acc->active_conns, 1);
            free(inner);
            free(w);
            close(cfd);
        }
    }
    return NULL;
}