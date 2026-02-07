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

int main() {

    /* Registra la funzione di cleanup da eseguire alla terminazione */
    atexit(server_shutdown_cleanup);
    
    /* Carica i messaggi persistenti nella cache in memoria */
    int loaded = load_messages_from_file();
    printf("[Server] Caricati %d messaggi nella cache di memoria.\n", loaded);

    /* Installa l'handler per la gestione del segnale SIGINT (Ctrl+C) */
    signal(SIGINT, handle_sigint);

    struct sockaddr_in addr = {0};
    pthread_t tid;

    /* Assicura che il file degli utenti esista */
    FILE *fp = fopen(USER_FILE, "a");
    if (fp) fclose(fp);

    /* Crea il socket del server */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Permette il riutilizzo immediato della porta dopo la chiusura */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Inizializza la struttura dell'indirizzo del server */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    /* Associa il socket alla porta specificata */
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Mette il socket in ascolto per connessioni in ingresso */
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("[Server] Avviato su porta %d\n", PORT);

    /* Loop principale di accettazione delle connessioni */
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        /* Accetta una nuova connessione client */
        int client_socket = accept(server_fd,
                                   (struct sockaddr *)&client_addr,
                                   &client_len);

        /* Gestione corretta degli errori di accept */
        if (client_socket < 0) {
            if (!running)
                break;              /* Terminazione richiesta (SIGINT) */
            if (errno == EINTR)
                continue;           /* Segnale innocuo: riprova accept */
            perror("accept");
            continue;
        }

        /* Alloca e inizializza la struttura del client */
        ClientHandler *client = malloc(sizeof(ClientHandler));
        if (!client) {
            perror("malloc");
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->username[0] = '\0';

        /* Crea il thread dedicato alla gestione del client */
        if (pthread_create(&tid, NULL, handle_client, client) != 0) {
            perror("pthread_create");
            close(client->socket);
            free(client);
        } else {
            /* Il thread viene automaticamente rilasciato alla fine */
            pthread_detach(tid);
        }
    }

    /* Terminazione del server (cleanup gestito da atexit) */
    return 0;
}
