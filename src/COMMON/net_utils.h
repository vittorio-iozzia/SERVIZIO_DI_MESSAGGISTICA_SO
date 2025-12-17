#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <sys/types.h>
#include <stddef.h>

ssize_t recv_line(int sock, char *buffer, size_t size);

#endif
