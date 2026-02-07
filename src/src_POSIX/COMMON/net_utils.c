#include "net_utils.h"
#include <sys/socket.h>

/*
 * Riceve una riga di testo da una socket TCP.
 *
 * Legge un byte alla volta fino al carattere di newline '\n'
 * oppure fino al raggiungimento della dimensione massima del buffer.
 *
 * Valore di ritorno:
 *  >0  numero di byte letti (newline escluso)
 *   0  connessione chiusa dal peer
 *  -1  errore di ricezione
 */
ssize_t recv_line(int sock, char *buffer, size_t size) {

    size_t i = 0;
    char c;

    /* Lettura carattere per carattere */
    while (i < size - 1) {

        ssize_t r = recv(sock, &c, 1, 0);

        if (r < 0)
            return -1;          /* Errore di ricezione */

        if (r == 0)
            return 0;           /* Connessione chiusa */

        if (c == '\n') {        /* Fine linea */
            buffer[i] = '\0';
            return (ssize_t)i;
        }

        buffer[i++] = c;
    }

    /* Buffer pieno: terminazione di sicurezza */
    buffer[i] = '\0';
    return (ssize_t)i;
}
