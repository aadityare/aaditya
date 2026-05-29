#pragma once

typedef struct { int fd; } HTTPArg;

void *handle_http(void *arg);
