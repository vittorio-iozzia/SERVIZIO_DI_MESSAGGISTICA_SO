#include "auth.h"
#include "net_utils.h"
#include "input_utils.h"
#include "common.h"

#include <stdio.h>
#include <string.h>

#include <winsock2.h>

/**
 * @brief Gestisce l'autenticazione o la registrazione del client.
 *
 * Mostra un menu testuale che consente all'utente di:
 *  - effettuare il login
 *  - registrarsi
 *  - uscire dal client
 *
 * I comandi vengono inviati al server tramite protocollo testuale.
 *
 * @param sock Socket TCP già connessa al server.
 * @return int 0 se l'autenticazione ha successo, -1 altrimenti.
 */
int client_authenticate(socket_t sock) {

    char buffer[MAX_MSG_LEN];
    char user[MAX_USERNAME_LEN];
    char pass[MAX_PASSWORD_LEN];

    /* Loop finché l'utente non si autentica o decide di uscire */
    while (1) {

        printf("\n1. Login\n2. Registrazione\n3. Esci\n> ");

        char choice[10];
        if (!fgets(choice, sizeof(choice), stdin))
            return -1;

        /* Uscita volontaria */
        if (choice[0] == '3')
            return -1;

        /* Inserimento username */
        printf("Username: ");
        if (!fgets(user, sizeof(user), stdin))
            return -1;
        clean_input(user);

        /* Inserimento password */
        printf("Password: ");
        if (!fgets(pass, sizeof(pass), stdin))
            return -1;
        clean_input(pass);

        /* Costruzione del comando secondo il protocollo */
        if (choice[0] == '1') {
            snprintf(buffer, sizeof(buffer),
                     "LOGIN|%s|%s\n", user, pass);
        } else if (choice[0] == '2') {
            snprintf(buffer, sizeof(buffer),
                     "REGISTER|%s|%s\n", user, pass);
        } else {
            continue;
        }

        /* Invio richiesta al server */
        send(sock, buffer, (int)strlen(buffer), 0);

        /* Attesa risposta dal server */
        if (recv_line(sock, buffer, sizeof(buffer)) <= 0)
            return -1;

        /* Autenticazione riuscita */
        if (strcmp(buffer, "OK") == 0) {
            printf("Login effettuato con successo!\n");
            return 0;
        }

        /* Messaggio di errore o notifica dal server */
        printf("Server: %s\n", buffer);
    }
}
