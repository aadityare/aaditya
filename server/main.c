// serves:
//   :22   SSH banner-peek, proxy to Go SSH server on 127.0.0.1:2222
//   :23   Telnet TUI
//   :8000 HTTP (Caddy proxies aadi.zip here; curl=ANSI, browser=HTML)
// compile:
//   gcc -O2 -Wall -Wextra -lpthread \
//       server/main.c \
//       server/data.c \
//       server/util/io.c \
//       server/util/strings.c \
//       server/net/listener.c \
//       server/net/acceptor.c \
//       server/proto/ssh.c \
//       server/proto/telnet.c \
//       server/proto/http.c \
//       server/content/http_cli.c \
//       server/content/static_files.c \
//       -o server/server

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include "config.h"
#include "util/strings.h"
#include "net/listener.h"
#include "net/acceptor.h"
#include "proto/ssh.h"
#include "proto/telnet.h"
#include "proto/http.h"

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    strings_load();

    pid_t pid = fork();
    if (pid == 0) {
        execl("./ssh-server/ssh-server", "ssh-server", NULL);
        perror("[server] failed to start ssh-server");
        exit(1);
    } else if (pid > 0) {
        printf("[server] spawned ssh-server (pid %d)\n", pid);
        sleep(1);
    }

    int ssh_fd    = make_listener(SSH_PUBLIC_PORT);
    int telnet_fd = make_listener(TELNET_PORT);
    int http_fd   = make_listener(HTTP_PORT);

    printf("[server] ssh    :%d -> 127.0.0.1:%d (Go)\n",
           SSH_PUBLIC_PORT, SSH_INTERNAL_PORT);
    printf("[server] telnet :%d (C)\n", TELNET_PORT);
    printf("[server] http   :%d (C)\n", HTTP_PORT);
    printf("[server] ready.\n");

    static Acceptor ssh_acc    = { .handler = handle_ssh,    .arg_size = sizeof(SSHArg)    };
    static Acceptor telnet_acc = { .handler = handle_telnet, .arg_size = sizeof(TelnetArg) };
    static Acceptor http_acc   = { .handler = handle_http,   .arg_size = sizeof(HTTPArg)   };
    ssh_acc.listen_fd    = ssh_fd;
    telnet_acc.listen_fd = telnet_fd;
    http_acc.listen_fd   = http_fd;

    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, accept_loop, &ssh_acc);
    pthread_create(&t2, NULL, accept_loop, &telnet_acc);
    pthread_create(&t3, NULL, accept_loop, &http_acc);

    pthread_join(t1, NULL);
    return 0;
}
