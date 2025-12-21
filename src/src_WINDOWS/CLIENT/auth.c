#include "auth.h"
#include "net_utils.h"
#include "input_utils.h"
#include "common.h"

#include <stdio.h>
#include <string.h>

#include <winsock2.h>


/**
 * @brief Gestisce l'autenticazione o registrazione del client.
 *
 * La funzione mostra un menu testuale che permette all'utente
 * di effettuare il login, registrarsi oppure uscire. I comandi
 * vengono inviati al server tramite protocollo testuale.
 *
 * @param sock Socket connesso al server.
 * @return int 0 se l'autenticazione ha successo, -1 altrimenti.
 */
int client_authenticate(socket_t sock) {

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
        if (!fgets(user, sizeof(user), stdin))
            return -1;
        clean_input(user);

        printf("Password: ");
        if (!fgets(pass, sizeof(pass), stdin))
            return -1;
        clean_input(pass);

        if (choice[0] == '1') {
            snprintf(buffer, sizeof(buffer),
                     "LOGIN|%s|%s\n", user, pass);
        } else if (choice[0] == '2') {
            snprintf(buffer, sizeof(buffer),
                     "REGISTER|%s|%s\n", user, pass);
        } else {
            continue;
        }

        send(sock, buffer, (int)strlen(buffer), 0);

        if (recv_line(sock, buffer, sizeof(buffer)) <= 0)
            return -1;

        if (strcmp(buffer, "OK") == 0) {
            printf("Login effettuato con successo!\n");
            return 0;
        }

        printf("Server: %s\n", buffer);
    }
}
