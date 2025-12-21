# --------------------- COMPILERS ---------------------

CC_POSIX   = gcc
CC_WINDOWS = x86_64-w64-mingw32-gcc

# --------------------- FLAGS ---------------------

CFLAGS_COMMON = -Wall -Wextra -std=c11

CFLAGS_POSIX   = $(CFLAGS_COMMON) -pthread
CFLAGS_WINDOWS = $(CFLAGS_COMMON)

LDFLAGS_POSIX   = -pthread
LDFLAGS_WINDOWS = -lws2_32

# --------------------- PATHS ---------------------

POSIX_DIR   = src_POSIX
WINDOWS_DIR = src_WINDOWS

SERVER_DIR  = SERVER
CLIENT_DIR  = CLIENT
COMMON_DIR  = COMMON

# --------------------- SOURCES ---------------------

POSIX_SERVER_SRC = \
	$(POSIX_DIR)/SERVER/main.c \
	$(POSIX_DIR)/SERVER/server.c \
	$(POSIX_DIR)/SERVER/messages.c \
	$(POSIX_DIR)/SERVER/users.c \
	$(POSIX_DIR)/COMMON/net_utils.c

POSIX_CLIENT_SRC = \
	$(POSIX_DIR)/CLIENT/main.c \
	$(POSIX_DIR)/CLIENT/client.c \
	$(POSIX_DIR)/CLIENT/auth.c \
	$(POSIX_DIR)/CLIENT/menu.c \
	$(POSIX_DIR)/CLIENT/input_utils.c \
	$(POSIX_DIR)/COMMON/net_utils.c

WINDOWS_SERVER_SRC = \
	$(WINDOWS_DIR)/SERVER/main.c \
	$(WINDOWS_DIR)/SERVER/server.c \
	$(WINDOWS_DIR)/SERVER/messages.c \
	$(WINDOWS_DIR)/SERVER/users.c \
	$(WINDOWS_DIR)/COMMON/net_utils.c

WINDOWS_CLIENT_SRC = \
	$(WINDOWS_DIR)/CLIENT/main.c \
	$(WINDOWS_DIR)/CLIENT/client.c \
	$(WINDOWS_DIR)/CLIENT/auth.c \
	$(WINDOWS_DIR)/CLIENT/menu.c \
	$(WINDOWS_DIR)/CLIENT/input_utils.c \
	$(WINDOWS_DIR)/COMMON/net_utils.c

# --------------------- INCLUDES ---------------------

POSIX_INCLUDES = \
	-I$(POSIX_DIR)/SERVER \
	-I$(POSIX_DIR)/CLIENT \
	-I$(POSIX_DIR)/COMMON

WINDOWS_INCLUDES = \
	-I$(WINDOWS_DIR)/SERVER \
	-I$(WINDOWS_DIR)/CLIENT \
	-I$(WINDOWS_DIR)/COMMON

# --------------------- TARGETS ---------------------

.PHONY: all posix windows clean

all:
	@echo "Usa 'make posix' oppure 'make windows'"

# --------------------- POSIX ---------------------

posix: posix_server posix_client

posix_server:
	$(CC_POSIX) $(CFLAGS_POSIX) $(POSIX_SERVER_SRC) \
	$(POSIX_INCLUDES) $(LDFLAGS_POSIX) \
	-o server.exe

posix_client:
	$(CC_POSIX) $(CFLAGS_POSIX) $(POSIX_CLIENT_SRC) \
	$(POSIX_INCLUDES) $(LDFLAGS_POSIX) \
	-o client.exe

# --------------------- WINDOWS ---------------------

windows: windows_server windows_client

windows_server:
	$(CC_WINDOWS) $(CFLAGS_WINDOWS) $(WINDOWS_SERVER_SRC) \
	$(WINDOWS_INCLUDES) $(LDFLAGS_WINDOWS) \
	-o server.exe

windows_client:
	$(CC_WINDOWS) $(CFLAGS_WINDOWS) $(WINDOWS_CLIENT_SRC) \
	$(WINDOWS_INCLUDES) $(LDFLAGS_WINDOWS) \
	-o client.exe

# --------------------- CLEAN ---------------------

clean:
	rm -f server.exe client.exe
