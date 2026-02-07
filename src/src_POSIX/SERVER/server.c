#include "server.h"
#include "users.h"
#include "messages.h"
#include "../COMMON/net_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

/* ----------------- VARIABILI GLOBALI ----------------- */

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

MessageCache global_cache = {NULL, 0, 0};

int server_fd = -1;
volatile sig_atomic_t running = 1;

/* ----------------- SIGNAL HANDLER ----------------- */

void handle_sigint(int sig) {
    (void)sig;
    running = 0;
    if (server_fd > 0)
        close(server_fd);
}

/* ----------------- CLEANUP ----------------- */

void server_shutdown_cleanup() {
    printf("[Cleanup] Salvataggio stato della cache su disco...\n");
    save_messages_to_file();

    printf("[Cleanup] Rilascio memoria cache...\n");
    pthread_mutex_lock(&cache_mutex);
    if (global_cache.array != NULL) {
        free(global_cache.array);
        global_cache.array = NULL;
    }
    pthread_mutex_unlock(&cache_mutex);

    printf("[Cleanup] Server terminato.\n");
}

/* ----------------- CLIENT HANDLER ----------------- */

void *handle_client(void *arg) {
    ClientHandler *client = (ClientHandler *)arg;
    int sock = client->socket;
    char buffer[MAX_MSG_LEN];

    /* -------- AUTENTICAZIONE / REGISTRAZIONE -------- */

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t r = recv_line(sock, buffer, sizeof(buffer));
        if (r <= 0) {
            close(sock);
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

    /* -------- LOOP PRINCIPALE -------- */

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv_line(sock, buffer, sizeof(buffer));
        if (bytes <= 0)
            break;

        char *cmd = strtok(buffer, "|");
        if (!cmd)
            continue;

        /* ----- SEND ----- */
        if (strcmp(cmd, "SEND") == 0) {

            char *to   = strtok(NULL, "|");
            char *subj = strtok(NULL, "|");
            char *body = strtok(NULL, "");

            if (!to || !subj || !body) {
                send(sock, "ERR\n", 4, 0);
                continue;
            }

            /* CONTROLLO COERENZA: destinatario registrato */
            if (!user_exists(to)) {
                send(sock, "ERR_NO_USER\n", 12, 0);
                continue;
            }

            save_message(client->username, to, subj, body);
            send(sock, "OK\n", 3, 0);

        /* ----- READ ----- */
        } else if (strcmp(cmd, "READ") == 0) {

            read_messages(client->username, sock);

        /* ----- DELETE ----- */
        } else if (strcmp(cmd, "DELETE") == 0) {

            delete_messages(client->username);
            send(sock, "OK\n", 3, 0);

        /* ----- QUIT ----- */
        } else if (strcmp(cmd, "QUIT") == 0) {

            send(sock, "BYE\n", 4, 0);
            break;

        } else {
            send(sock, "ERR\n", 4, 0);
        }
    }

    printf("[Server] Utente '%s' disconnesso.\n", client->username);

    close(sock);
    free(client);
    return NULL;
}
