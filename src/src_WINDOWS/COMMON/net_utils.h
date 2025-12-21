#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <stddef.h>
#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Riceve una riga terminata da '\n' da un socket.
 *
 * La funzione legge un carattere alla volta fino a incontrare
 * il carattere di newline oppure fino a esaurimento del buffer.
 *
 * @param sock Socket da cui leggere.
 * @param buffer Buffer di destinazione.
 * @param size Dimensione del buffer.
 * @return ssize_t Numero di byte letti (>0), 0 se connessione chiusa,
 *                 -1 in caso di errore.
 */
ssize_t recv_line(socket_t sock, char *buffer, size_t size);


#ifdef __cplusplus
}
#endif

#endif /* NET_UTILS_H */
