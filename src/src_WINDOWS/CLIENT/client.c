#include "client.h"
#include "auth.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "common.h"


#define PORT 8080


/**
 * @brief Avvia il programma client.
 *
 * La funzione inizializza Winsock, crea una connessione TCP
 * verso il server locale e avvia la fase di autenticazione.
 * Dopo l'autenticazione, delega la gestione delle operazioni
 * al menu principale del client.
 */
void client_program(void) {

    socket_t sock;
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup fallita\n");
        exit(EXIT_FAILURE);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port   = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server.\n");

    if (client_authenticate(sock) != 0) {
        closesocket(sock);
        WSACleanup();
        return;
    }

    client_menu_loop(sock);

    closesocket(sock);
    WSACleanup();
}
