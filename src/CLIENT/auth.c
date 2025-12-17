#include "auth.h"
#include "../COMMON/net_utils.h"
#include "input_utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../COMMON/common.h"

int client_authenticate(int sock) {
    char buffer[MAX_MSG_LEN];
    char user[MAX_USERNAME_LEN];
    char pass[MAX_PASSWORD_LEN];

    while (1) {
        printf("\n1. Login\n2. Registrazione\n3. Esci\n> ");
        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin))
            return -1;

        if (choice[0] == '3')
            return -1;

        printf("Username: ");
        fgets(user, sizeof(user), stdin);
        clean_input(user);

        printf("Password: ");
        fgets(pass, sizeof(pass), stdin);
        clean_input(pass);

        if (choice[0] == '1')
            snprintf(buffer, sizeof(buffer), "LOGIN|%s|%s\n", user, pass);
        else if (choice[0] == '2')
            snprintf(buffer, sizeof(buffer), "REGISTER|%s|%s\n", user, pass);
        else
            continue;

        send(sock, buffer, strlen(buffer), 0);

        if (recv_line(sock, buffer, sizeof(buffer)) <= 0)
            return -1;

        if (strcmp(buffer, "OK") == 0) {
            printf("Login effettuato con successo!\n");
            return 0;
        }

        printf("Server: %s\n", buffer);
    }
}
