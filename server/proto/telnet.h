#pragma once

typedef struct { int fd; } TelnetArg;

void *handle_telnet(void *arg);
