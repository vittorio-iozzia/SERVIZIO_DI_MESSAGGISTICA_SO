#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include "../COMMON/common.h"

#define USER_FILE "DATA/users.txt"
#define MSG_FILE "DATA/messages.txt"

/* ---------------- STRUTTURE ---------------- */

typedef struct {
    int id;
    char recipient[50];
    char sender[50];
    char subject[100];
    char body[1024];
} Message;

typedef struct {
    Message *array;
    size_t count;
    size_t capacity;
} MessageCache;

typedef struct {
    int socket;
    char username[50];
} ClientHandler;

/* ---------------- VARIABILI GLOBALI ---------------- */
/* Definite in server.c */

extern pthread_mutex_t file_mutex;
extern pthread_mutex_t cache_mutex;
extern pthread_mutex_t client_count_mutex;

extern MessageCache global_cache;
extern int server_fd;
extern volatile sig_atomic_t running;
extern int active_clients;

extern pthread_t active_threads[MAX_CLIENTS];
extern int client_sockets[MAX_CLIENTS];
extern pthread_mutex_t client_count_mutex;
extern int active_clients;

/* Aggiunta della variabile per il thread di flush, necessaria al main per la pthread_join */
extern pthread_t flush_thread_id; 

/* ---------------- PROTOTIPI FUNZIONI ---------------- */

void server_shutdown_cleanup();
void handle_sigint(int sig);
void *handle_client(void *arg);
void start_flush_thread();

#endif