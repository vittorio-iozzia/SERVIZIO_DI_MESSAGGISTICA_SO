#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <sys/types.h>
#include <stddef.h>

/* Riceve una riga di testo da una socket TCP fino al newline '\n' */
ssize_t recv_line(int sock, char *buffer, size_t size);

/* Invia tutto il buffer sulla socket TCP in modo sicuro.
 * Usa MSG_NOSIGNAL per evitare SIGPIPE.
 *
 * Ritorna:
 *  0  -> successo
 * -1  -> errore
 */
int send_all(int sock, const char *buf, size_t len);

#endif /* NET_UTILS_H */
