#pragma once

typedef struct { int fd; } SSHArg;

void *handle_ssh(void *arg);
