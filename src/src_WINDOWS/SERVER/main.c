#include "server.h"
#include "messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>   /* _beginthreadex */


/**
 * @brief Wrapper per l'avvio di un thread client su Windows.
 *
 * La funzione handle_client ha una firma POSIX-like
 * (void *(*)(void *)), mentre _beginthreadex richiede
 * una funzione con firma unsigned __stdcall.
 * Questo wrapper permette di adattare correttamente
 * la funzione handle_client all'API Windows.
 *
 * @param arg Puntatore alla struttura ClientHandler.
 * @return unsigned Valore di ritorno del thread (0).
 */
unsigned __stdcall client_thread_wrapper(void *arg) {
    handle_client(arg);
    return 0;
}


/**
 * @brief Handler per gli eventi di controllo della console.
 *
 * Intercetta eventi come CTRL+C o la chiusura della finestra
 * della console e avvia la procedura di terminazione del server
 * tramite handle_sigint().
 *
 * @param signal Codice dell'evento di console.
 * @return TRUE se l'evento è stato gestito, FALSE altrimenti.
 */
BOOL WINAPI console_ctrl_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        handle_sigint(0);
        return TRUE;
    }
    return FALSE;
}


/**
 * @brief Funzione main del server (versione Windows).
 *
 * La funzione inizializza l'ambiente Winsock, configura
 * le strutture di sincronizzazione, carica la cache dei messaggi
 * da disco e avvia un server TCP concorrente basato su thread.
 *
 * Il server resta in esecuzione finché la variabile globale
 * `running` rimane vera. Alla terminazione, le operazioni di
 * cleanup vengono eseguite automaticamente tramite atexit().
 *
 * @return int Codice di terminazione del programma.
 */
int main(void) {

    /* Registrazione della funzione di cleanup */
    atexit(server_shutdown_cleanup);

    /* Inizializzazione di Winsock */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup fallita\n");
        exit(EXIT_FAILURE);
    }

    /* Inizializzazione delle mutex */
    InitializeCriticalSection(&file_mutex);
    InitializeCriticalSection(&cache_mutex);

    /* Installazione dell'handler per CTRL+C */
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

    /* Caricamento dei messaggi dalla cache persistente */
    int loaded = load_messages_from_file();
    printf("[Server] Caricati %d messaggi nella cache.\n", loaded);

    /* Preparazione del socket di ascolto */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    /* Assicura l'esistenza del file utenti */
    FILE *fp = fopen(USER_FILE, "a");
    if (fp) fclose(fp);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               (char *)&opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[Server] Avviato su porta %d\n", PORT);

    /* Ciclo principale di accettazione delle connessioni */
    while (running) {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        socket_t client_socket =
            accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        /*
         * Evita la creazione di thread inutili durante
         * la fase di terminazione del server.
         */
        if (!running) {
            if (client_socket != INVALID_SOCKET) {
                closesocket(client_socket);
            }
            break;
        }

        if (client_socket == INVALID_SOCKET) {
            continue;
        }

        ClientHandler *client = malloc(sizeof(ClientHandler));
        if (!client) {
            closesocket(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->username[0] = '\0';

        uintptr_t th = _beginthreadex(
            NULL,
            0,
            client_thread_wrapper,
            client,
            0,
            NULL
        );

        if (th == 0) {
            closesocket(client_socket);
            free(client);
        } else {
            CloseHandle((HANDLE)th);
        }
    }

    /* Il cleanup viene eseguito automaticamente da atexit() */
    return 0;
}
