# --------------------- COMPILERS ---------------------

CC_POSIX   = gcc
CC_WINDOWS = x86_64-w64-mingw32-gcc

# --------------------- FLAGS ---------------------

CFLAGS_COMMON = -Wall -Wextra -g -std=c11

CFLAGS_POSIX   = $(CFLAGS_COMMON) -pthread
CFLAGS_WINDOWS = $(CFLAGS_COMMON)

LDFLAGS_POSIX   = -pthread
LDFLAGS_WINDOWS = -lws2_32

# --------------------- PATHS ---------------------

POSIX_DIR   = src_POSIX
WINDOWS_DIR = src_WINDOWS

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

# --------------------- EXECUTABLES ---------------------

TARGET_SERVER_POSIX = server_posix
TARGET_CLIENT_POSIX = client_posix
TARGET_SERVER_WIN   = server_win.exe
TARGET_CLIENT_WIN   = client_win.exe

# --------------------- TARGETS ---------------------

.PHONY: all posix windows clean

all:
	@echo "Usa 'make posix' oppure 'make windows'"

# --------------------- POSIX ---------------------

posix: $(TARGET_SERVER_POSIX) $(TARGET_CLIENT_POSIX)

$(TARGET_SERVER_POSIX): $(POSIX_SERVER_SRC)
	$(CC_POSIX) $(CFLAGS_POSIX) $(POSIX_SERVER_SRC) $(POSIX_INCLUDES) $(LDFLAGS_POSIX) -o $@

$(TARGET_CLIENT_POSIX): $(POSIX_CLIENT_SRC)
	$(CC_POSIX) $(CFLAGS_POSIX) $(POSIX_CLIENT_SRC) $(POSIX_INCLUDES) $(LDFLAGS_POSIX) -o $@

# --------------------- WINDOWS ---------------------

windows: $(TARGET_SERVER_WIN) $(TARGET_CLIENT_WIN)

$(TARGET_SERVER_WIN): $(WINDOWS_SERVER_SRC)
	$(CC_WINDOWS) $(CFLAGS_WINDOWS) $(WINDOWS_SERVER_SRC) $(WINDOWS_INCLUDES) $(LDFLAGS_WINDOWS) -o $@

$(TARGET_CLIENT_WIN): $(WINDOWS_CLIENT_SRC)
	$(CC_WINDOWS) $(CFLAGS_WINDOWS) $(WINDOWS_CLIENT_SRC) $(WINDOWS_INCLUDES) $(LDFLAGS_WINDOWS) -o $@

# --------------------- CLEAN ---------------------

clean:
	rm -f $(TARGET_SERVER_POSIX) $(TARGET_CLIENT_POSIX) \
	      $(TARGET_SERVER_WIN) $(TARGET_CLIENT_WIN) \
	      *.tmp