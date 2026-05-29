#include "telnet.h"
#include "../config.h"
#include "../ansi.h"
#include "../data.h"
#include "../util/io.h"
#include "../util/strings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

static const unsigned char IAC_SETUP[] = {
    255, 251, 3,
    255, 253, 3,
};

static void strip_iac(unsigned char *buf, int *len) {
    int i = 0, out = 0;
    while (i < *len) {
        if (buf[i] == 255 && i + 1 < *len) {
            unsigned char cmd = buf[i + 1];
            i += (cmd >= 251 && cmd <= 254) ? 3 : 2;
        } else {
            buf[out++] = buf[i++];
        }
    }
    *len = out;
}

static void telnet_home(int fd) {
    send_str(fd, "\r\n");
    send_str(fd, CYN "  aaditya rengarajan\r\n" R);
    send_str(fd, DIM "  cybersecurity & ai researcher\r\n" R);
    send_str(fd, "\r\n");
    send_str(fd, "  ms cybersecurity, new york university.\r\n");
    send_str(fd, "  i work at the crossroads of offensive security,\r\n");
    send_str(fd, "  agentic ai systems, and large-scale automation.\r\n");
    send_str(fd, "\r\n");
    send_str(fd, DIM "  formerly at intel, isro, isac, and equate petrochemical.\r\n" R);
    send_str(fd, "\r\n");
    send_str(fd, "  i've contributed to gov-grade threat intelligence platforms,\r\n");
    send_str(fd, "  deep rl research for intel foundry ops, and led cybersec\r\n");
    send_str(fd, "  education for 500+ learners.\r\n");
    send_str(fd, g_telnet_prompt);
}

static void telnet_research(int fd) {
    send_str(fd, "\r\n");
    send_str(fd, CYN "  research & publications\r\n" R);
    send_str(fd, DIM "  ----------------------------------------------------------\r\n" R);
    send_str(fd, "\r\n");
    char buf[256];
    for (size_t i = 0; i < num_papers; i++) {
        snprintf(buf, sizeof(buf), "  [%zu] %s\r\n", i + 1, papers[i].title);
        send_str(fd, buf);
        snprintf(buf, sizeof(buf), DIM "       %s\r\n\r\n" R, papers[i].doi);
        send_str(fd, buf);
    }
    send_str(fd, "\r\n----------------------------------------------------------\r\n");
    send_str(fd, "type 1-5 to read abstract   home  about  rps  quit\r\n> ");
}

static void telnet_paper(int fd, size_t idx) {
    char buf[512];
    send_str(fd, "\r\n");
    snprintf(buf, sizeof(buf), CYN "  %s\r\n" R, papers[idx].title);
    send_str(fd, buf);
    send_str(fd, DIM "  ----------------------------------------------------------\r\n\r\n" R);
    send_str(fd, "  abstract\r\n\r\n");
    send_wrapped(fd, papers[idx].abstract, 60, "  ");
    send_str(fd, "\r\n");
    snprintf(buf, sizeof(buf), "  doi: " GRN "%s\r\n" R, papers[idx].doi);
    send_str(fd, buf);
    send_str(fd, "\r\n----------------------------------------------------------\r\n");
    send_str(fd, "home  research  about  rps  quit\r\n> ");
}

static void telnet_about(int fd) {
    send_str(fd, "\r\n");
    send_str(fd, CYN "  about me\r\n" R);
    send_str(fd, DIM "  ----------------------------------------------------------\r\n\r\n" R);
    char buf[256];
    for (size_t i = 0; i < num_links; i++) {
        snprintf(buf, sizeof(buf), "  %-12s  %s\r\n",
                 about_links[i].label, about_links[i].sub);
        send_str(fd, buf);
        snprintf(buf, sizeof(buf), GRN "              %s\r\n\r\n" R, about_links[i].url);
        send_str(fd, buf);
    }
    send_str(fd, "----------------------------------------------------------\r\n");
    send_str(fd, "home  research  rps  quit\r\n> ");
}

static const char *rps_choices[] = {"rock", "paper", "scissors"};

static void telnet_rps(int fd) {
    send_str(fd, "\r\n");
    send_str(fd, CYN "  rock paper scissors\r\n" R);
    send_str(fd, DIM "  the machine decides.\r\n\r\n" R);
    for (int i = 3; i >= 1; i--) {
        char buf[16];
        snprintf(buf, sizeof(buf), "  %d...\r\n", i);
        send_str(fd, buf);
        sleep(1);
    }
    /* FIX: seed with time ^ thread id, not address of a local variable */
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)pthread_self());
    const char *choice = rps_choices[rand() % 3];
    size_t len = strlen(choice);

    send_str(fd, "\r\n  the machine chose:\r\n\r\n  ");
    for (size_t i = 0; i < len; i++) {
        char c[2] = { choice[i], '\0' };
        send_str(fd, c);
        if (i < len - 1) {
            struct timespec ts = { .tv_sec = 0, .tv_nsec = 110000000 };
            nanosleep(&ts, NULL);
        }
    }
    send_str(fd, "\r\n");
    send_str(fd, "\r\n----------------------------------------------------------\r\n");
    send_str(fd, "play again? rps   or: home  research  about  quit\r\n> ");
}

void *handle_telnet(void *arg) {
    TelnetArg *a = arg;
    int fd = a->fd;
    free(a);
    pthread_detach(pthread_self());

    write_all(fd, (const char*)IAC_SETUP, sizeof(IAC_SETUP));
    telnet_home(fd);

    unsigned char line[LINE_MAX_LEN + 1];
    int line_len = 0;

    while (1) {
        unsigned char tmp[256];
        ssize_t n = read(fd, tmp, sizeof(tmp));
        if (n <= 0) break;
        int ilen = (int)n;
        strip_iac(tmp, &ilen);

        for (int i = 0; i < ilen; i++) {
            unsigned char c = tmp[i];

            if (c == '\r' || c == '\n') {
                line[line_len] = '\0';

                char *cmd = (char*)line;
                while (*cmd == ' ') cmd++;
                char *end = cmd + strlen(cmd) - 1;
                while (end > cmd && *end == ' ') *end-- = '\0';

                for (char *p = cmd; *p; p++)
                    if (*p >= 'A' && *p <= 'Z') *p += 32;

                if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit") || !strcmp(cmd, "q")) {
                    send_str(fd, "\r\n  goodbye!\r\n\r\n");
                    goto done;
                } else if (!strcmp(cmd, "") || !strcmp(cmd, "home")) {
                    telnet_home(fd);
                } else if (!strcmp(cmd, "research")) {
                    telnet_research(fd);
                } else if (!strcmp(cmd, "about")) {
                    telnet_about(fd);
                } else if (!strcmp(cmd, "rps")) {
                    telnet_rps(fd);
                } else if (cmd[0] >= '1' && cmd[0] <= '0' + (char)num_papers && cmd[1] == '\0') {
                    size_t idx = (size_t)(cmd[0] - '1');
                    if (idx < num_papers) telnet_paper(fd, idx);
                } else {
                    char err[128];
                    snprintf(err, sizeof(err),
                        "\r\n  unknown: '%s'\r\n"
                        "  try: home  research  about  rps  quit\r\n> ", cmd);
                    send_str(fd, err);
                }
                line_len = 0;

            } else if (c == 127 || c == 8) {
                if (line_len > 0) line_len--;
            } else if (c >= 32 && c < 127 && line_len < LINE_MAX_LEN) {
                line[line_len++] = c;
            }
        }
    }
done:
    close(fd);
    return NULL;
}
