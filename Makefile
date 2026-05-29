CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -Wpedantic -std=c11 -D_GNU_SOURCE
LDFLAGS = -lpthread

C_SRCS = \
    server/main.c \
    server/data.c \
    server/util/io.c \
    server/util/strings.c \
    server/net/listener.c \
    server/net/acceptor.c \
    server/proto/ssh.c \
    server/proto/telnet.c \
    server/proto/http.c \
    server/content/http_cli.c \
    server/content/static_files.c

C_OUT   = server/server
GO_DIR  = ssh-server
GO_OUT  = ssh-server/ssh-server
HOST_KEY = ssh-server/host_key

.PHONY: all c go key clean

all: key go c

## C server
c: $(C_OUT)
$(C_OUT): $(C_SRCS)
	$(CC) $(CFLAGS) $(C_SRCS) $(LDFLAGS) -o $(C_OUT)

## Go SSH server
go: $(GO_OUT)
$(GO_OUT):
	cd $(GO_DIR) && go mod tidy && go build -o ssh-server .

## Host key (only if missing)
key:
	@if [ ! -f $(HOST_KEY) ]; then \
	    echo "[keygen] generating ed25519 host key..."; \
	    ssh-keygen -t ed25519 -f $(HOST_KEY) -N "" -q; \
	    echo "[keygen] done: $(HOST_KEY)"; \
	else \
	    echo "[keygen] host key already exists, skipping."; \
	fi

clean:
	rm -f $(C_OUT) $(GO_OUT)
