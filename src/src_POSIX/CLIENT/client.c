#include "client.h"
#include "auth.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../COMMON/common.h"

#define PORT 8080

/*
 * Avvia il programma client.
 *
 * Si occupa di:
 *  - creare la socket TCP
 *  - connettersi al server locale
 *  - gestire l'autenticazione dell'utente
 *  - avviare il menu interattivo principale
 */
void client_program(void) {

    int sock;
    struct sockaddr_in server = {0};

    /* Evita la terminazione del processo in caso di scrittura su socket chiusa */
    signal(SIGPIPE, SIG_IGN);

    /* Creazione socket TCP */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Configurazione indirizzo del server */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    /* Connessione al server */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server.\n");

    /* Fase di autenticazione */
    if (client_authenticate(sock) != 0) {
        close(sock);
        return;
    }

    /* Avvio del menu interattivo del client */
    client_menu_loop(sock);

    /* Chiusura socket */
    close(sock);
}
