#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include "../COMMON/common.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>


/**
 * @brief Tipo astratto per i socket.
 *
 * Su Windows i socket sono rappresentati dal tipo SOCKET.
 */
typedef SOCKET socket_t;


/**
 * @brief Tipo astratto per la mutua esclusione.
 *
 * L’implementazione utilizza le Critical Section di Windows.
 */
typedef CRITICAL_SECTION mutex_t;


/**
 * @brief Tipo atomico per il controllo dello stato del server.
 *
 * Utilizzato per segnalare la terminazione del server
 * in modo thread-safe.
 */
typedef volatile LONG atomic_int_t;


/* Configurazione del server */
#define PORT 8080
#define USER_FILE "DATA/users.txt"
#define MSG_FILE  "DATA/messages.txt"


/**
 * @brief Struttura che rappresenta un singolo messaggio.
 */
typedef struct {
    char recipient[50];
    char sender[50];
    char subject[100];
    char body[1024];
} Message;


/**
 * @brief Cache dei messaggi residente in memoria.
 */
typedef struct {
    Message *array;
    size_t count;
    size_t capacity;
} MessageCache;


/**
 * @brief Struttura associata a un client connesso.
 */
typedef struct {
    socket_t socket;
    char username[50];
} ClientHandler;


/* Variabili globali definite in server.c */
extern mutex_t file_mutex;
extern mutex_t cache_mutex;
extern MessageCache global_cache;
extern socket_t server_fd;
extern atomic_int_t running;


/**
 * @brief Esegue il cleanup finale del server.
 */
void server_shutdown_cleanup(void);


/**
 * @brief Gestisce la richiesta di terminazione del server.
 *
 * Su Windows viene invocata dal ConsoleCtrlHandler
 * registrato nel main.
 *
 * @param sig Parametro non utilizzato.
 */
void handle_sigint(int sig);


/**
 * @brief Gestisce la comunicazione con un singolo client.
 *
 * La funzione viene eseguita all’interno di un thread
 * dedicato creato con _beginthreadex.
 *
 * @param arg Puntatore a ClientHandler.
 * @return void* Sempre NULL.
 */
void *handle_client(void *arg);

#endif /* SERVER_H */
