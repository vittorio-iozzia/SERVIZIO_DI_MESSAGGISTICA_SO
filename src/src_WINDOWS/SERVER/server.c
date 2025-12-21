#include "server.h"
#include "users.h"
#include "messages.h"
#include "../COMMON/net_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <windows.h>


/* Variabili globali definite dall'interfaccia (server.h) */
mutex_t file_mutex;
mutex_t cache_mutex;
MessageCache global_cache = { NULL, 0, 0 };
socket_t server_fd = INVALID_SOCKET;
atomic_int_t running = 1;


/**
 * @brief Gestisce la richiesta di terminazione del server.
 *
 * Imposta la variabile globale running a 0 e chiude il socket
 * di ascolto per sbloccare l'eventuale accept() in corso.
 *
 * Nota: su Windows questa funzione viene invocata dal handler
 * della console (CTRL+C) installato nel main.
 *
 * @param sig Parametro non utilizzato (compatibilità).
 */
void handle_sigint(int sig) {
    (void)sig;

    running = 0;

    if (server_fd != INVALID_SOCKET) {
        closesocket(server_fd);
        server_fd = INVALID_SOCKET;
    }
}


/**
 * @brief Esegue le operazioni di cleanup alla terminazione del server.
 *
 * Salva lo stato della cache dei messaggi su disco e rilascia la memoria
 * allocata. È pensata per essere registrata tramite atexit().
 *
 * Questa funzione non chiude Winsock e non distrugge le CriticalSection:
 * tali operazioni dipendono dalla strategia adottata nel main/relazione.
 * Se vuoi, possiamo centralizzare anche quelle qui.
 */
void server_shutdown_cleanup(void) {

    printf("[Cleanup] Salvataggio stato della cache su disco...\n");
    save_messages_to_file();

    printf("[Cleanup] Rilascio memoria cache...\n");

    EnterCriticalSection(&cache_mutex);
    if (global_cache.array) {
        free(global_cache.array);
        global_cache.array = NULL;
        global_cache.count = 0;
        global_cache.capacity = 0;
    }
    LeaveCriticalSection(&cache_mutex);

    printf("[Cleanup] Server terminato.\n");
}


/**
 * @brief Gestisce un singolo client connesso.
 *
 * Esegue autenticazione/registrazione tramite protocollo testuale
 * e poi gestisce i comandi principali (SEND/READ/DELETE/QUIT).
 *
 * @param arg Puntatore a ClientHandler allocato dinamicamente.
 * @return void* Sempre NULL (firma compatibile con codice preesistente).
 */
void *handle_client(void *arg) {

    ClientHandler *client = (ClientHandler *)arg;
    socket_t sock = client->socket;
    char buffer[MAX_MSG_LEN];

    while (running) {

        memset(buffer, 0, sizeof(buffer));
        ssize_t r = recv_line(sock, buffer, sizeof(buffer));

        if (r <= 0) {
            closesocket(sock);
            free(client);
            return NULL;
        }

        char *cmd  = strtok(buffer, "|");
        char *user = strtok(NULL, "|");
        char *pass = strtok(NULL, "|");

        if (!cmd || !user || !pass) {
            send(sock, "ERR\n", 4, 0);
            continue;
        }

        if (strcmp(cmd, "LOGIN") == 0) {

            if (authenticate(user, pass)) {
                strncpy(client->username, user, sizeof(client->username) - 1);
                client->username[sizeof(client->username) - 1] = '\0';
                send(sock, "OK\n", 3, 0);
                break;
            } else {
                send(sock, "FAIL\n", 5, 0);
            }

        } else if (strcmp(cmd, "REGISTER") == 0) {

            if (create_user(user, pass)) {
                send(sock, "OK_REG\n", 7, 0);
            } else {
                send(sock, "FAIL_EXISTS\n", 12, 0);
            }

        } else {
            send(sock, "ERR\n", 4, 0);
        }
    }

    printf("[Server] Utente '%s' connesso.\n", client->username);

    while (running) {

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv_line(sock, buffer, sizeof(buffer));
        if (bytes <= 0) {
            break;
        }

        char *cmd = strtok(buffer, "|");
        if (!cmd) continue;

        if (strcmp(cmd, "SEND") == 0) {

            char *to   = strtok(NULL, "|");
            char *subj = strtok(NULL, "|");
            char *body = strtok(NULL, "");

            if (to && subj && body) {
                save_message(client->username, to, subj, body);
                send(sock, "OK\n", 3, 0);
            } else {
                send(sock, "ERR\n", 4, 0);
            }

        } else if (strcmp(cmd, "READ") == 0) {

            read_messages(client->username, sock);

        } else if (strcmp(cmd, "DELETE") == 0) {

            delete_messages(client->username);
            send(sock, "OK\n", 3, 0);

        } else if (strcmp(cmd, "QUIT") == 0) {

            send(sock, "BYE\n", 4, 0);
            break;

        } else {
            send(sock, "ERR\n", 4, 0);
        }
    }

    printf("[Server] Utente '%s' disconnesso.\n", client->username);

    closesocket(sock);
    free(client);
    return NULL;
}
