#include "io.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int write_all(int fd, const char *buf, size_t n) {
    size_t sent = 0;
    while (sent < n) {
        ssize_t r = write(fd, buf + sent, n - sent);
        if (r <= 0) return -1;
        sent += (size_t)r;
    }
    return 0;
}

int send_str(int fd, const char *s) {
    return write_all(fd, s, strlen(s));
}

void send_wrapped(int fd, const char *text, int width, const char *indent) {
    char line[512];
    int indent_len = (int)strlen(indent);
    snprintf(line, sizeof(line), "%s", indent);
    int line_len = indent_len;

    const char *p = text;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;

        const char *word_start = p;
        while (*p && *p != ' ') p++;
        int word_len = (int)(p - word_start);

        if (line_len + word_len + 1 > width && line_len > indent_len) {
            strncat(line, "\r\n", sizeof(line) - strlen(line) - 1);
            send_str(fd, line);
            snprintf(line, sizeof(line), "%s", indent);
            line_len = indent_len;
        }
        if (line_len > indent_len) {
            strncat(line, " ", sizeof(line) - strlen(line) - 1);
            line_len++;
        }
        int space = (int)(sizeof(line) - strlen(line) - 1);
        if (word_len > space) word_len = space;
        strncat(line, word_start, (size_t)word_len);
        line_len += word_len;
    }
    if (line_len > indent_len) {
        strncat(line, "\r\n", sizeof(line) - strlen(line) - 1);
        send_str(fd, line);
    }
}
