#include "strings.h"
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *g_sitemap       = NULL;
char *g_telnet_prompt = NULL;

static char *load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); exit(1); }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { perror("malloc"); exit(1); }
    size_t got = fread(buf, 1, (size_t)sz, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

void strings_load(void) {
    char path[PATH_MAX_LEN];

    snprintf(path, sizeof(path), "%s/sitemap.txt",       STRINGS_DIR);
    g_sitemap = load_file(path);

    snprintf(path, sizeof(path), "%s/telnet_prompt.txt", STRINGS_DIR);
    g_telnet_prompt = load_file(path);
}
