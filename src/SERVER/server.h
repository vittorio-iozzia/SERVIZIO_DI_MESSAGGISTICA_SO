#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include "../COMMON/common.h"

#define PORT 8080
#define USER_FILE "../DATA/users.txt"
#define MSG_FILE "../DATA/messages.txt"

/* Strutture e variabili globali */
typedef struct {
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

/* Variabili globali (definite in server.c) */
extern pthread_mutex_t file_mutex;
extern pthread_mutex_t cache_mutex;
extern MessageCache global_cache;
extern int server_fd;
extern volatile sig_atomic_t running;

/* Prototypes */
void server_shutdown_cleanup();
void handle_sigint(int sig);
void *handle_client(void *arg);

#endif
