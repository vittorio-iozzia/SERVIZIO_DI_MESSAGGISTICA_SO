#include "net_utils.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>

/* Fallback per macOS/BSD dove MSG_NOSIGNAL non è definito */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/*
 * Riceve una riga di testo da una socket TCP in modo sicuro.
 *
 * - Legge un byte alla volta fino al newline '\n'.
 * - Ignora i carriage return '\r' (compatibilità CRLF).
 * - Previene la desincronizzazione: se la riga supera la dimensione
 * del buffer, tronca la lettura ma "svuota" il socket fino al '\n'
 * per evitare che la riga successiva venga corrotta.
 *
 * Valore di ritorno:
 * >0  numero di byte validi letti
 * 0  connessione chiusa in modo pulito
 * -1  errore (o timeout del socket)
 */
ssize_t recv_line(int sock, char *buffer, size_t size) {
    size_t i = 0;
    char c = 0;

    if (size <= 1) return -1;

    while (i < size - 1) {
        ssize_t r = recv(sock, &c, 1, 0);

        if (r < 0) {
            if (errno == EINTR) continue; // Segnale asincrono, riprova
            
            /* Tracciamento esplicito del timeout impostato dal server/client */
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Il timeout è scattato, disconnettiamo il client inattivo
                return -1;
            }
            perror("recv");
            return -1;
        }

        if (r == 0) {
            /* Connessione chiusa dal peer */
            if (i == 0) return 0; 
            break; /* Avevamo letto qualcosa prima della chiusura anomala */
        }

        if (c == '\n') {
            break; /* Fine riga trovata */
        }

        /* Ignoro i '\r' per non sporcare le stringhe lette */
        if (c != '\r') {
            buffer[i++] = c;
        }
    }

    buffer[i] = '\0';

    /* PREVENZIONE DESYNC DEL PROTOCOLLO
       Se sono uscito dal while perché il buffer è pieno (i == size - 1)
       ma non ho ancora visto il '\n', il resto della riga corromperà
       la successiva recv. Devo "svuotare" il resto della riga.
    */
    if (i == size - 1 && c != '\n') {
        char discard;
        while (1) {
            ssize_t dr = recv(sock, &discard, 1, 0);
            if (dr > 0) {
                if (discard == '\n') break;
            } else if (dr < 0) {
                if (errno == EINTR) continue; // Segnale asincrono, riprova
                /* Se c'è un timeout o errore critico MENTRE scartiamo i byte
                   in eccesso, la socket è desincronizzata. Dobbiamo forzare la chiusura. */
                return -1; 
            } else {
                /* Anche in caso di EOF improvviso durante lo scarto */
                return -1; 
            }
        }
    }

    return (ssize_t)i;
}

/*
 * Invia tutto il buffer sulla socket TCP in modo sicuro.
 * Usa MSG_NOSIGNAL per evitare il fatale SIGPIPE.
 *
 * Ritorna:
 * 0  -> successo
 * -1  -> errore o connessione interrotta
 */
int send_all(int sock, const char *buf, size_t len) {
    size_t total = 0;

    while (total < len) {
        /* MSG_NOSIGNAL impedisce al sistema operativo
           di inviare SIGPIPE se il client ha disconnesso brutalmente la presa. 
           Invece di crashare, send ritorna -1 e imposta errno a EPIPE. */
        ssize_t n = send(sock, buf + total, len - total, MSG_NOSIGNAL);

        if (n < 0) {
            if (errno == EINTR) continue; // ritenta se interrotto da segnale
            // Non stampiamo perror su EPIPE per evitare spam nei log se il client cade
            if (errno != EPIPE) perror("send"); 
            return -1;
        }

        if (n == 0) {
            /* Tecnicamente impossibile su socket bloccanti in Linux, ma buona pratica */
            return -1;
        }

        total += n;
    }

    return 0;
}