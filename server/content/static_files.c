#include "static_files.h"
#include "../config.h"
#include "../util/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <limits.h>

static void http_404(int fd) {
    send_str(fd,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 9\r\n"
        "Connection: close\r\n\r\n"
        "Not Found");
}

static void serve_file(int fd, const char *path, const char *mime) {
    int f = open(path, O_RDONLY);
    if (f < 0) { http_404(fd); return; }
    struct stat st;
    if (fstat(f, &st) < 0 || !S_ISREG(st.st_mode)) {
        close(f); http_404(fd); return;
    }
    char hdr[512];
    snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n",
        mime, (long)st.st_size);
    send_str(fd, hdr);
    sendfile(fd, f, NULL, (size_t)st.st_size);
    close(f);
}

static const char *mime_for(const char *ext) {
    if (!ext)                           return "application/octet-stream";
    if (!strcmp(ext, ".html"))          return "text/html; charset=utf-8";
    if (!strcmp(ext, ".css"))           return "text/css";
    if (!strcmp(ext, ".js"))            return "application/javascript";
    if (!strcmp(ext, ".pdf"))           return "application/pdf";
    if (!strcmp(ext, ".png"))           return "image/png";
    if (!strcmp(ext, ".jpg")
     || !strcmp(ext, ".jpeg"))          return "image/jpeg";
    if (!strcmp(ext, ".svg"))           return "image/svg+xml";
    if (!strcmp(ext, ".ico"))           return "image/x-icon";
    if (!strcmp(ext, ".woff2"))         return "font/woff2";
    if (!strcmp(ext, ".txt"))           return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

static void chunk(int fd, const char *s) {
    size_t n = strlen(s);
    char hdr[24];
    snprintf(hdr, sizeof(hdr), "%zx\r\n", n);
    send_str(fd, hdr);
    send_str(fd, s);
    send_str(fd, "\r\n");
}

static void chunk_end(int fd) { send_str(fd, "0\r\n\r\n"); }

static void chunk_escaped(int fd, const char *s) {
    char buf[2] = {0, 0};
    char acc[512];
    size_t acc_len = 0;

#define FLUSH_ACC() do { \
    if (acc_len) { acc[acc_len] = '\0'; chunk(fd, acc); acc_len = 0; } \
} while(0)

    for (; *s; s++) {
        const char *esc = NULL;
        switch (*s) {
            case '&': esc = "&amp;";  break;
            case '<': esc = "&lt;";   break;
            case '>': esc = "&gt;";   break;
            case '"': esc = "&quot;"; break;
        }
        if (esc) {
            FLUSH_ACC();
            chunk(fd, esc);
        } else {
            if (acc_len >= sizeof(acc) - 1) FLUSH_ACC();
            acc[acc_len++] = *s;
        }
    }
    FLUSH_ACC();
    (void)buf;
#undef FLUSH_ACC
}

static void serve_dirlist(int fd, const char *uri_path, const char *dir_path) {
    DIR *d = opendir(dir_path);
    if (!d) { http_404(fd); return; }

    struct dirent *ent;
    int count = 0;
    while ((ent = readdir(d)) != NULL)
        if (ent->d_name[0] != '.') count++;

    char **names = malloc((size_t)count * sizeof(char *));
    if (!names) { closedir(d); http_404(fd); return; }

    rewinddir(d);
    int i = 0;
    while ((ent = readdir(d)) != NULL && i < count)
        if (ent->d_name[0] != '.') names[i++] = strdup(ent->d_name);
    count = i;
    closedir(d);

    for (int a = 0; a < count - 1; a++)
        for (int b = a + 1; b < count; b++)
            if (strcmp(names[a], names[b]) > 0) {
                char *tmp = names[a]; names[a] = names[b]; names[b] = tmp;
            }

    send_str(fd,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: close\r\n\r\n");

    chunk(fd,
        "<!DOCTYPE html><html lang=en><head>"
        "<meta charset=UTF-8>"
        "<meta name=viewport content='width=device-width,initial-scale=1'>"
        "<link rel=stylesheet href='/styles.css'/>"
        "</head><body><div class='indexor'>");

    {
        char h[PATH_MAX + 32];
        snprintf(h, sizeof(h), "<h1>%s</h1>", uri_path);
        chunk(fd, h);
    }

    if (strcmp(uri_path, "/") != 0) {
        char tmp[PATH_MAX];
        strncpy(tmp, uri_path, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        size_t len = strlen(tmp);
        if (len > 1 && tmp[len-1] == '/') tmp[--len] = '\0';
        char *slash = strrchr(tmp, '/');
        if (slash) {
            slash[1] = '\0';
            char parent[PATH_MAX + 64];
            snprintf(parent, sizeof(parent),
                "<a href='%s'><span class=dim>../</span></a>", tmp);
            chunk(fd, parent);
        }
    }

    for (int j = 0; j < count; j++) {
        char fullpath[PATH_MAX + NAME_MAX + 2];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, names[j]);
        struct stat st;
        int is_dir = (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode));

        char href[PATH_MAX * 2];
        size_t ulen = strlen(uri_path);
        snprintf(href, sizeof(href), "%s%s%s%s",
                 uri_path,
                 (ulen > 0 && uri_path[ulen-1] == '/') ? "" : "/",
                 names[j],
                 is_dir ? "/" : "");

        char tag[PATH_MAX * 2 + 32];
        snprintf(tag, sizeof(tag),
                 "<a href='%s'%s>", href, is_dir ? " class=dir" : "");
        chunk(fd, tag);
        chunk_escaped(fd, names[j]);
        if (is_dir) chunk(fd, "/");
        chunk(fd, "</a>");

        free(names[j]);
    }
    free(names);

    chunk(fd, "</div></body></html>");
    chunk_end(fd);
}

void serve_static(int fd, const char *uri) {
    char clean[URI_MAX_LEN + 1];
    strncpy(clean, uri, URI_MAX_LEN);
    clean[URI_MAX_LEN] = '\0';
    char *q = strchr(clean, '?');
    if (q) *q = '\0';

    char path[PATH_MAX_LEN];
    int written = snprintf(path, sizeof(path), "%s%s", STATIC_DIR, clean);
    if (written < 0 || (size_t)written >= sizeof(path)) {
        http_404(fd); return;
    }

    char resolved[PATH_MAX];
    char root[PATH_MAX];
    if (!realpath(STATIC_DIR, root))  { http_404(fd); return; }
    if (!realpath(path, resolved))    { http_404(fd); return; }

    size_t root_len = strlen(root);
    if (strncmp(resolved, root, root_len) != 0 ||
        (resolved[root_len] != '/' && resolved[root_len] != '\0')) {
        http_404(fd); return;
    }

    struct stat st;
    if (stat(resolved, &st) < 0) { http_404(fd); return; }

    if (S_ISDIR(st.st_mode)) {
        char idx[PATH_MAX + 12];
        snprintf(idx, sizeof(idx), "%s/index.html", resolved);
        if (access(idx, R_OK) == 0) {
            serve_file(fd, idx, "text/html; charset=utf-8");
            return;
        }
        size_t clen = strlen(clean);
        if (clen == 0 || clean[clen-1] != '/') {
            char loc[PATH_MAX + 86];
            snprintf(loc, sizeof(loc),
                "HTTP/1.1 301 Moved Permanently\r\n"
                "Location: %s/\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n\r\n", clean);
            send_str(fd, loc);
            return;
        }
        serve_dirlist(fd, clean, resolved);
        return;
    }

    if (!S_ISREG(st.st_mode)) { http_404(fd); return; }
    serve_file(fd, resolved, mime_for(strrchr(resolved, '.')));
}

void serve_index(int fd) {
    char path[PATH_MAX_LEN];
    snprintf(path, sizeof(path), "%s/index.html", STATIC_DIR);
    serve_file(fd, path, "text/html; charset=utf-8");
}