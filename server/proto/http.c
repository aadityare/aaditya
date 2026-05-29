#include "http.h"
#include "../config.h"
#include "../util/io.h"
#include "../content/http_cli.h"
#include "../content/static_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

static const char *CLI_AGENTS[] = {
    "curl/", "Wget/", "HTTPie/", "httpie/",
    "python-httpx", "python-requests", "Go-http-client", NULL
};

static int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static int is_cli(const char *ua) {
    if (!ua || !*ua) return 1;
    for (int i = 0; CLI_AGENTS[i]; i++)
        if (starts_with(ua, CLI_AGENTS[i])) return 1;
    return 0;
}

void *handle_http(void *arg) {
    HTTPArg *a = arg;
    int fd = a->fd;
    free(a);
    pthread_detach(pthread_self());

    char req[BUF];
    ssize_t n = recv(fd, req, sizeof(req) - 1, 0);
    if (n <= 0) { close(fd); return NULL; }
    req[n] = '\0';

    char method[16], uri[URI_MAX_LEN + 1];
    if (sscanf(req, "%15s %511s", method, uri) != 2) {
        close(fd); return NULL;
    }
    (void)method;

    char ua[UA_MAX_LEN + 1] = {0};
    char *ua_hdr = strcasestr(req, "\r\nUser-Agent:");
    if (ua_hdr) {
        ua_hdr += 13; /* skip "\r\nUser-Agent:" */
        while (*ua_hdr == ' ') ua_hdr++;
        char *end = strstr(ua_hdr, "\r\n");
        int ulen = end ? (int)(end - ua_hdr) : (int)strlen(ua_hdr);
        if (ulen > UA_MAX_LEN) ulen = UA_MAX_LEN;
        strncpy(ua, ua_hdr, (size_t)ulen);
        ua[ulen] = '\0';
    }

    int cli = is_cli(ua);

    char *qs = strchr(uri, '?');
    if (qs) *qs = '\0';

    if (!strcmp(uri, "/") || !strcmp(uri, "")) {
        cli ? curl_home(fd)     : serve_index(fd);
    } else if (!strcmp(uri, "/research")) {
        cli ? curl_research(fd) : serve_index(fd);
    } else if (!strcmp(uri, "/about")) {
        cli ? curl_about(fd)    : serve_index(fd);
    } else if (!strcmp(uri, "/help")) {
        cli ? curl_help(fd)     : serve_index(fd);
    } else {
        serve_static(fd, uri);
    }

    close(fd);
    return NULL;
}
