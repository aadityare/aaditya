#pragma once
#include <time.h>
#include <stddef.h>
#include <stdint.h>

#define RATE_BUCKETS    1024 
#define RATE_TOKENS     10
#define RATE_REFILL_SEC 1

typedef struct {
    uint32_t  ip;
    int       tokens;
    time_t    last_refill;
} RateBucket;

#define MAX_CONNS_PER_PORT  200

typedef struct {
    int    listen_fd;
    void *(*handler)(void *);
    size_t arg_size;

    RateBucket rate_table[RATE_BUCKETS];

    volatile int active_conns;
} Acceptor;

void *accept_loop(void *arg);