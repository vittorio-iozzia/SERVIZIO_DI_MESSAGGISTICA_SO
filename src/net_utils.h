// src/net_utils.h
#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <sys/types.h> // Per ssize_t

/**
 * @brief Legge una riga da un socket, carattere per carattere, fino al terminatore '\n'.
 * * @param sock Il file descriptor del socket.
 * @param buffer Il buffer dove memorizzare i dati.
 * @param size La dimensione massima del buffer.
 * @return ssize_t Il numero di byte letti (incluso '\n'), o <= 0 in caso di errore/chiusura.
 */
ssize_t recv_line(int sock, char *buffer, size_t size);

#endif