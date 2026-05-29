#pragma once
#include <stddef.h>

typedef struct { const char *title; const char *abstract; const char *doi; } Paper;
typedef struct { const char *label; const char *sub;      const char *url; } Link;

extern Paper  papers[];
extern size_t num_papers;

extern Link   about_links[];
extern size_t num_links;
