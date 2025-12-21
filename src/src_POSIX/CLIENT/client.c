#include "client.h"
#include "auth.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../COMMON/common.h"

#define PORT 8080

void client_program(void) {
    int sock;
    struct sockaddr_in server = {0};

    signal(SIGPIPE, SIG_IGN);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server.\n");

    if (client_authenticate(sock) != 0) {
        close(sock);
        return;
    }

    client_menu_loop(sock);

    close(sock);
}
