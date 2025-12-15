// src/net_utils.c
#include "net_utils.h"
#include <unistd.h>
#include <sys/socket.h>

ssize_t recv_line(int sock, char *buffer, size_t size) {
    size_t i = 0;
    char c;
    while (i < size - 1) {
        ssize_t r = recv(sock, &c, 1, 0);
        if (r < 0) return -1; // Errore
        if (r == 0) return 0; // Connessione chiusa
        
        if (c == '\n') {
            buffer[i] = '\0'; // Termina la stringa
            return i + 1;     // Ritorna il conteggio reale dei byte letti
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}