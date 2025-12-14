// server_linux.c
// Server TCP concorrente minimale (base progetto messaggistica)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int server_fd;

/* ==================== SIGNAL HANDLER ==================== */

void handle_sigint(int sig) {
    printf("\n[Server] Chiusura server...\n");
    if (server_fd > 0)
        close(server_fd);
    exit(0);
}

/* ==================== CLIENT HANDLER ==================== */

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    send(client_socket, "Server pronto.\n", 15, 0);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0)
            break;

        if (strncmp(buffer, "QUIT", 4) == 0)
            break;

        send(client_socket, buffer, strlen(buffer), 0); // echo
    }

    close(client_socket);
    printf("[Server] Client disconnesso.\n");
    return NULL;
}

/* ==================== MAIN ==================== */

int main() {
    signal(SIGINT, handle_sigint);

    struct sockaddr_in server_addr = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[Server] Avviato sulla porta %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int *client_socket = malloc(sizeof(int));
        if (!client_socket)
            continue;

        *client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (*client_socket < 0) {
            free(client_socket);
            if (errno == EINTR)
                continue;
            perror("accept");
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_socket) != 0) {
            perror("pthread_create");
            close(*client_socket);
            free(client_socket);
        } else {
            pthread_detach(tid);
        }
    }

    return 0;
}
