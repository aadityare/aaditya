#include "http_cli.h"
#include "../ansi.h"
#include "../data.h"
#include "../util/io.h"
#include "../util/strings.h"
#include <stdio.h>
#include <string.h>

static void http_200_text(int fd) {
    send_str(fd,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: close\r\n\r\n");
}

static void chunk(int fd, const char *data) {
    size_t n = strlen(data);
    char hdr[32];
    snprintf(hdr, sizeof(hdr), "%zx\r\n", n);
    send_str(fd, hdr);
    write_all(fd, data, n);
    send_str(fd, "\r\n");
}

static void chunk_end(int fd) { send_str(fd, "0\r\n\r\n"); }

void curl_home(int fd) {
    http_200_text(fd);
    chunk(fd, "\r\n");
    chunk(fd, CYN BOLD "  aaditya rengarajan\r\n" R);
    chunk(fd, DIM "  cybersecurity & ai researcher\r\n\r\n" R);
    chunk(fd, "  ms cybersecurity, new york university.\r\n");
    chunk(fd, "  i work at the crossroads of offensive security,\r\n");
    chunk(fd, "  agentic ai systems, and large-scale automation.\r\n\r\n");
    chunk(fd, DIM "  formerly at intel, isro, isac, and equate petrochemical.\r\n\r\n" R);
    chunk(fd, "  i've contributed to gov-grade threat intelligence platforms,\r\n");
    chunk(fd, "  deep rl research for intel foundry ops, and led cybersec\r\n");
    chunk(fd, "  education for 500+ learners.\r\n");
    chunk(fd, g_sitemap);
    chunk_end(fd);
}

void curl_research(int fd) {
    http_200_text(fd);
    chunk(fd, "\r\n");
    chunk(fd, CYN BOLD "  research & publications\r\n" R);
    chunk(fd, DIM "  ----------------------------------------------------------\r\n\r\n" R);
    char buf[512];
    for (size_t i = 0; i < num_papers; i++) {
        snprintf(buf, sizeof(buf), "  [%zu]  %s\r\n", i + 1, papers[i].title);
        chunk(fd, buf);
        snprintf(buf, sizeof(buf), DIM "        %s\r\n\r\n" R, papers[i].doi);
        chunk(fd, buf);
    }
    chunk(fd, g_sitemap);
    chunk_end(fd);
}

void curl_about(int fd) {
    http_200_text(fd);
    chunk(fd, "\r\n");
    chunk(fd, CYN BOLD "  about me\r\n" R);
    chunk(fd, DIM "  ----------------------------------------------------------\r\n\r\n" R);
    char buf[256];
    for (size_t i = 0; i < num_links; i++) {
        snprintf(buf, sizeof(buf), "  %-12s  %s\r\n",
                 about_links[i].label, about_links[i].sub);
        chunk(fd, buf);
        snprintf(buf, sizeof(buf), GRN "              %s\r\n\r\n" R,
                 about_links[i].url);
        chunk(fd, buf);
    }
    chunk(fd, g_sitemap);
    chunk_end(fd);
}

void curl_help(int fd) {
    http_200_text(fd);
    chunk(fd, "\r\n");
    chunk(fd, CYN BOLD "  aadi.zip \xe2\x80\x94 navigation\r\n" R);
    chunk(fd, DIM "  ----------------------------------------------------------\r\n\r\n" R);
    chunk(fd, g_sitemap);
    chunk_end(fd);
}
