#pragma once
#include <stddef.h>

int write_all(int fd, const char *buf, size_t n);

int send_str(int fd, const char *s);

void send_wrapped(int fd, const char *text, int width, const char *indent);
