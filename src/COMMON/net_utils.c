#include "net_utils.h"
#include <sys/socket.h>

ssize_t recv_line(int sock, char *buffer, size_t size) {
    size_t i = 0;
    char c;
    while (i < size - 1) {
        ssize_t r = recv(sock, &c, 1, 0);
        if (r < 0) return -1;
        if (r == 0) return 0;
        if (c == '\n') {
            buffer[i] = '\0';
            return (ssize_t)i;
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return (ssize_t)i;
}