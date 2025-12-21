#include "net_utils.h"

#include <winsock2.h>


/**
 * @brief Riceve una riga terminata da '\n' da un socket.
 *
 * La funzione legge il socket un byte alla volta fino a incontrare
 * il carattere di newline oppure fino al riempimento del buffer.
 *
 * @param sock Socket connesso.
 * @param buffer Buffer di destinazione.
 * @param size Dimensione massima del buffer.
 * @return ssize_t Numero di byte letti (>0), 0 se connessione chiusa,
 *                 -1 in caso di errore.
 */
ssize_t recv_line(socket_t sock, char *buffer, size_t size) {

    size_t i = 0;
    char c;

    while (i < size - 1) {

        int r = recv(sock, &c, 1, 0);

        if (r < 0)
            return -1;

        if (r == 0)
            return 0;

        if (c == '\n') {
            buffer[i] = '\0';
            return (ssize_t)i;
        }

        buffer[i++] = c;
    }

    buffer[i] = '\0';
    return (ssize_t)i;
}
