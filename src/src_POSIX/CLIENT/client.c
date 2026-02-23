#include "client.h"
#include "auth.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "../COMMON/common.h"

#define CLIENT_TIMEOUT 15

/*
 * Avvia il programma client.
 *
 * Architettura "Thin Client":
 * Il client si limita a instaurare la connessione e a fare da passacarte
 * tra l'utente e il server. Tutta la logica, la validazione e la sicurezza 
 * risiedono lato server.
 * La funzione ora accetta l'IP del server come parametro.
 */
void client_program(const char *server_ip) {
    int sock;
    struct sockaddr_in server = {0};

    /* Creazione socket TCP */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Portabilità macOS/BSD contro SIGPIPE. 
       Se compiliamo su Mac, MSG_NOSIGNAL non esiste. Usiamo SO_NOSIGPIPE. */
#ifdef __APPLE__
    int set_option = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &set_option, sizeof(set_option)) < 0) {
        perror("setsockopt SO_NOSIGPIPE");
        // Non fatale, ma da loggare
    }
#endif

    /* Impostazione Timeout sul Socket. Un client robusto non si blocca mai all'infinito. */
    struct timeval tv;
    tv.tv_sec = CLIENT_TIMEOUT;
    tv.tv_usec = 0;
    
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_RCVTIMEO");
        exit(EXIT_FAILURE); /* Errore fatale: non vogliamo un client che possa bloccarsi */
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_SNDTIMEO");
        exit(EXIT_FAILURE);
    }

    /* Configurazione indirizzo del server */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    
    /*  Utilizzo dell'IP passato come parametro invece di quello hardcodato */
    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    /* Connessione al server */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server.\n");

    /* Fase di autenticazione */
    if (client_authenticate(sock) != 0) {
        shutdown(sock, SHUT_RDWR); /* Forzo la chiusura della socket a livello di protocollo (abbatto le connessioni)*/
        close(sock);
        return;
    }

    /* Avvio del menu interattivo del client */
    client_menu_loop(sock);

    /* Chiusura socket in modo sicuro */
    shutdown(sock, SHUT_RDWR);
    close(sock);
}