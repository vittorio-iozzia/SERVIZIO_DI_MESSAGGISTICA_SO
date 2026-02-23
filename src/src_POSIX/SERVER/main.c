#define _DEFAULT_SOURCE
#include "server.h"
#include "messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

/* [MAC FIX] Fallback per i flag di rete se compilato su macOS o sistemi BSD */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

int main() {
    /* Inizializzazione array monitoraggio */
    pthread_mutex_lock(&client_count_mutex);
    memset(active_threads, 0, sizeof(active_threads));
    for(int i=0; i<MAX_CLIENTS; i++) client_sockets[i] = -1;
    pthread_mutex_unlock(&client_count_mutex);

    /* Carica i messaggi persistenti nella cache in memoria */
    int loaded = load_messages_from_file();
    printf("[Server] Caricati %d messaggi nella cache di memoria.\n", loaded);

    /* ---------------- INSTALLAZIONE SIGNAL HANDLER ---------------- */
    
    /* [PROTEZIONE GLOBALE SIGPIPE] 
       Evita il crash su macOS/BSD dove MSG_NOSIGNAL non esiste. 
       Su Linux agisce come ulteriore rete di sicurezza. */
    struct sigaction sa_pipe;
    memset(&sa_pipe, 0, sizeof(sa_pipe));
    sa_pipe.sa_handler = SIG_IGN;
    sigemptyset(&sa_pipe.sa_mask);
    sa_pipe.sa_flags = 0;
    sigaction(SIGPIPE, &sa_pipe, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {0};
    pthread_t tid;

    /* Assicura che il file degli utenti esista */
    FILE *fp = fopen(USER_FILE, "a");
    if (fp) fclose(fp);

    /* ---------------- CREAZIONE SOCKET SERVER ---------------- */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Modificato backlog da 10 a SOMAXCONN per gestire picchi di connessioni simultanee */
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("[Server] Avviato su porta %d\n", PORT);

    /* ---------------- AVVIO FLUSH PERIODICO ---------------- */
    start_flush_thread();  

    /* ---------------- LOOP PRINCIPALE ACCEPT ---------------- */
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0) {
            if (!running) break; 
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&client_count_mutex);
        if (active_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&client_count_mutex);
            /* Difesa DoS: MSG_DONTWAIT impedisce il blocco se il client malevolo non legge */
            send(client_socket, RESP_ERR "Server pieno\n", 18, MSG_NOSIGNAL | MSG_DONTWAIT);
            close(client_socket);
            continue;
        }

        /* Prenotazione sicura dello slot per evitare Race Condition tra client */
        int index = -1;
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == -1) { /* Controllo basato sui socket invece dei thread */
                index = i;
                client_sockets[index] = client_socket; /* Slot immediatamente prenotato! */
                break;
            }
        }

        active_clients++;
        pthread_mutex_unlock(&client_count_mutex);

        ClientHandler *client = malloc(sizeof(ClientHandler));
        if (!client) {
            perror("malloc client");
            close(client_socket);
            /* Rollback della prenotazione in caso di errore di malloc */
            pthread_mutex_lock(&client_count_mutex);
            client_sockets[index] = -1;
            active_clients--;
            pthread_mutex_unlock(&client_count_mutex);
            continue;
        }

        client->socket = client_socket;
        client->username[0] = '\0';

        if (pthread_create(&tid, NULL, handle_client, client) != 0) {
            perror("pthread_create");
            close(client->socket);
            free(client);
            /* Rollback della prenotazione se fallisce la creazione del thread */
            pthread_mutex_lock(&client_count_mutex);
            client_sockets[index] = -1;
            active_clients--;
            pthread_mutex_unlock(&client_count_mutex);
        } else {
            /* Aggiorniamo solo il TID, lo slot socket è già al sicuro */
            pthread_mutex_lock(&client_count_mutex);
            active_threads[index] = tid;
            pthread_mutex_unlock(&client_count_mutex);
        }
    }

    /* ---------------- SPEGNIMENTO CONTROLLATO ---------------- */
    printf("\n[Server] Arresto in corso... Sincronizzazione thread.\n");
    
    /* 1. Chiudo forzatamente i socket dei client per sbloccare le recv() appese */
    pthread_mutex_lock(&client_count_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {  /* Controllo sul socket è più sicuro */
            shutdown(client_sockets[i], SHUT_RDWR);
        }
    }
    pthread_mutex_unlock(&client_count_mutex);

    /* 2. Attendo che tutti i thread worker (ora distaccati) finiscano le operazioni.
          Uso il contatore in modo thread-safe invece di fare join. */
    while (1) {
        pthread_mutex_lock(&client_count_mutex);
        int remaining = active_clients;
        pthread_mutex_unlock(&client_count_mutex);
        
        if (remaining == 0) break;
        usleep(50000); /* Attende 50ms per non consumare CPU inutilmente */
    }

    /* 3. FERMO IL THREAD DI FLUSH PRIMA DI TOCCARE LA CACHE
          Questo evita Crash/Segmentation Fault fatali in fase di spegnimento */
    pthread_join(flush_thread_id, NULL);

    /* 4. Tutti i thread (worker e flush) sono chiusi: salvataggio finale e deallocazione */
    server_shutdown_cleanup();

    /* 5. Pulizia dei Mutex per rigore formale */
    pthread_mutex_destroy(&client_count_mutex);
    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&cache_mutex);

    printf("[Server] Chiusura pulita completata. Cache liberata.\n");
    return 0;
}