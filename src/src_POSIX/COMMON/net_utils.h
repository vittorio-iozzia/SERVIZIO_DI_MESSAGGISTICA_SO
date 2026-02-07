#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <sys/types.h>
#include <stddef.h>

/* Riceve una riga di testo da una socket TCP fino al newline */
ssize_t recv_line(int sock, char *buffer, size_t size);

#endif /* NET_UTILS_H */
