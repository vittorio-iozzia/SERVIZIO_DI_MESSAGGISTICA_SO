
#define _POSIX_C_SOURCE 200809L // Abilita pthread_sigmask e altre funzioni moderne
#include <signal.h>
#include <pthread.h>
#include "server.h"
#include "users.h"
#include "messages.h"
#include "../COMMON/net_utils.h"
#include "../COMMON/common.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>

/* ----------------- VARIABILI GLOBALI ----------------- */

MessageCache global_cache = {NULL, 0, 0};      
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;    
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;   
pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER; 

pthread_t active_threads[MAX_CLIENTS] = {0};
int client_sockets[MAX_CLIENTS] = {0};

int active_clients = 0;                         
int server_fd = -1;                             
volatile sig_atomic_t running = 1;              
pthread_t flush_thread_id;

#define FLUSH_INTERVAL 5          
#define CLIENT_TIMEOUT_SEC 120    

/* ----------------- SIGNAL HANDLER ----------------- */

void handle_sigint(int sig) {
    (void)sig;
    running = 0;  
    if (server_fd > 0)
        shutdown(server_fd, SHUT_RDWR); 
}

/* ----------------- FLUSH PERIODICO ----------------- */

void *flush_cache_periodically(void *arg) {
    (void)arg;

    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);                                     
    
    while (running) {
        for(int i = 0; i < FLUSH_INTERVAL && running; i++) {
            sleep(1);
        }
        if (running)
            save_messages_to_file();  
    }
    return NULL;
}

void start_flush_thread() {
    pthread_create(&flush_thread_id, NULL, flush_cache_periodically, NULL);
}

/* ----------------- CLEANUP SERVER ----------------- */

void server_shutdown_cleanup() {
    running = 0;
    
    /* 1. Salvataggio finale dei messaggi su disco */
    save_messages_to_file();
    
    /* 2. Deallocazione della cache in totale sicurezza. */
    pthread_mutex_lock(&cache_mutex);
    if (global_cache.array != NULL) {
        free(global_cache.array);
        global_cache.array = NULL;
        global_cache.count = 0;
        global_cache.capacity = 0;
    }
    pthread_mutex_unlock(&cache_mutex);
    
    printf("[Server] Memoria cache deallocata con successo.\n");
}

/* ----------------- CLIENT HANDLER ----------------- */

void *handle_client(void *arg) {
    ClientHandler *client = (ClientHandler *)arg;
    int sock = client->socket;
    char buffer[MAX_MSG_LEN];
    char *saveptr;

    /* [PREVENZIONE MEMORY LEAK] Distacco il thread. 
       Il sistema pulirà le risorse del thread in automatico appena esce. */
    pthread_detach(pthread_self());

    /* [THREAD-SAFETY] Blocco segnali nel worker per evitare interruzioni durante i lock */
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    /* [DoS PROTECTION] Timeout in RCV e SND per disconnettere client inattivi o malevoli */
    struct timeval tv;
    tv.tv_sec = CLIENT_TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    int login_attempts = 0;

    /* -------- FASE 1: AUTENTICAZIONE -------- */
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t r = recv_line(sock, buffer, sizeof(buffer));
        if (r <= 0) goto cleanup;

        char *cmd  = strtok_r(buffer, "|", &saveptr);
        if (!cmd) continue;

        if (strcmp(cmd, CMD_QUIT) == 0) {
            send_all(sock, RESP_BYE, strlen(RESP_BYE));
            goto cleanup;
        }

        char *user = strtok_r(NULL, "|", &saveptr);
        char *pass = strtok_r(NULL, "\n\r", &saveptr);

        if (!user || !pass) {
            send_all(sock, RESP_ERR_FORMAT, strlen(RESP_ERR_FORMAT));
            continue;
        }

        if (strlen(user) >= MAX_USERNAME_LEN || strlen(pass) >= MAX_PASSWORD_LEN) {
            send_all(sock, "ERR_CREDENTIALS_TOO_LONG\n", 25);
            continue;
        }

        if (strcmp(cmd, CMD_LOGIN) == 0) {
            if (authenticate(user, pass)) {
                strncpy(client->username, user, sizeof(client->username) - 1);
                client->username[sizeof(client->username) - 1] = '\0';
                send_all(sock, RESP_OK, strlen(RESP_OK));
                printf("[Server] Accesso riuscito: '%s'\n", client->username);
                break;
            } else {
                login_attempts++;
                if (login_attempts >= 3) {
                    send_all(sock, RESP_TOO_MANY, strlen(RESP_TOO_MANY));
                    goto cleanup;
                }
                send_all(sock, RESP_FAIL, strlen(RESP_FAIL));
            }
        } else if (strcmp(cmd, CMD_REGISTER) == 0) {
            if (create_user(user, pass)) {
                send_all(sock, RESP_OK_REG, strlen(RESP_OK_REG));
                printf("[Server] Registrato nuovo utente: '%s'\n", user);
            } else {
                send_all(sock, RESP_FAIL_EXISTS, strlen(RESP_FAIL_EXISTS));
            }
        } else {
            send_all(sock, RESP_ERR, strlen(RESP_ERR));
        }
    }

    /* -------- FASE 2: LOOP PRINCIPALE COMANDI -------- */
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv_line(sock, buffer, sizeof(buffer));
        if (bytes <= 0) break;

        char *cmd = strtok_r(buffer, "|", &saveptr);
        if (!cmd) continue;

        if (strcmp(cmd, CMD_SEND) == 0) {
            char *to   = strtok_r(NULL, "|", &saveptr);
            char *subj = strtok_r(NULL, "|", &saveptr);
            char *body = strtok_r(NULL, "\n\r", &saveptr);

            if (!to || !subj || !body) {
                send_all(sock, "ERR_MISSING_FIELDS\n", 19);
                continue;
            }

            /* [FIX DOs & Overflow] Controllo stringente sulle dimensioni dei campi */
            if (strlen(to) >= MAX_USERNAME_LEN || 
                strlen(subj) >= MAX_SUBJECT_LEN || 
                strlen(body) >= MAX_BODY_LEN) {
                
                send_all(sock, "ERR_MSG_TOO_LONG\n", 17);
                continue;
            }

            /* SANITIZZAZIONE INPUT */
            sanitize_input(to, MAX_USERNAME_LEN);
            sanitize_input(subj, MAX_SUBJECT_LEN);
            sanitize_input(body, MAX_BODY_LEN);

            if (!user_exists(to)) {
                send_all(sock, RESP_ERR_NO_USER, strlen(RESP_ERR_NO_USER));
                continue;
            }
            save_message(client->username, to, subj, body); 
            send_all(sock, RESP_OK, strlen(RESP_OK));

        } else if (strcmp(cmd, CMD_READ) == 0) {
            read_messages(client->username, sock);

        } else if (strcmp(cmd, CMD_DELETE) == 0) {
            delete_messages(client->username);
            send_all(sock, RESP_OK, strlen(RESP_OK));

        } else if (strcmp(cmd, CMD_DELETE_ONE) == 0) {
            char *id_str = strtok_r(NULL, "|", &saveptr);
            if (!id_str) {
                send_all(sock, RESP_ERR, strlen(RESP_ERR));
                continue;
            }
            
            char *endptr;
            long msg_id = strtol(id_str, &endptr, 10);
            if (endptr != id_str && msg_id > 0 && msg_id <= INT_MAX) {
                if (delete_specific_message(client->username, (int)msg_id))
                    send_all(sock, RESP_OK, strlen(RESP_OK));
                else
                    send_all(sock, RESP_ERR_NOT_FOUND, strlen(RESP_ERR_NOT_FOUND));
            } else {
                send_all(sock, "ERR_INVALID_ID\n", 15);
            }

        } else if (strcmp(cmd, CMD_QUIT) == 0) {
            send_all(sock, RESP_BYE, strlen(RESP_BYE));
            break;

        } else {
            send_all(sock, RESP_ERR, strlen(RESP_ERR));
        }
    }

cleanup:
    if (strlen(client->username) > 0) {
        printf("[Server] Logout: '%s'\n", client->username);
    } else {
        printf("[Server] Client non autenticato disconnesso.\n");
    }

    int my_sock = client->socket;
    free(client);

    /* [RIMOZIONE CON JOIN] Il thread si rimuove in modo sicuro dagli array */
    pthread_mutex_lock(&client_count_mutex);
    
    pthread_t self = pthread_self();
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (pthread_equal(active_threads[i], self)) {
            active_threads[i] = 0;
            client_sockets[i] = -1;
            break;
        }
    }
    
    close(my_sock);
    active_clients--;
    
    pthread_mutex_unlock(&client_count_mutex);

    return NULL;
}