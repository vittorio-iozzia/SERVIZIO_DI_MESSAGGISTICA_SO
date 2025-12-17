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
    atexit(server_shutdown_cleanup);
    
    int loaded = load_messages_from_file();
    printf("[Server] Caricati %d messaggi nella cache di memoria.\n", loaded);

    signal(SIGINT, handle_sigint);

    struct sockaddr_in addr = {0};
    pthread_t tid;

    /* Assicura che i file esistano */
    FILE *fp = fopen(USER_FILE, "a"); if (fp) fclose(fp);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket"); exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd); exit(EXIT_FAILURE);
    }
    
    printf("[Server] Avviato su porta %d\n", PORT);

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

        ClientHandler *client = malloc(sizeof(ClientHandler));
        if (!client) {
            perror("malloc");
            close(client_socket);
            continue;
        }

        client->socket = client_socket;
        client->username[0] = '\0';

        if (pthread_create(&tid, NULL, handle_client, client) != 0) {
            perror("pthread_create");
            close(client->socket);
            free(client);
        } else {
            pthread_detach(tid);
        }
    }

    return 0;
}
