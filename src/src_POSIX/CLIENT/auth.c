#include "auth.h"
#include "../COMMON/net_utils.h"
#include "input_utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "../COMMON/common.h"

/*  Aggiunta macro di utilità per confrontare in modo pulito le risposte del server
         ignorando il newline inserito dalle macro RESP_* di common.h */
#define MATCH_RESP(buf, resp_macro) \
    (strncmp((buf), (resp_macro), strlen(resp_macro) - 1) == 0 && (buf)[strlen(resp_macro) - 1] == '\0')

/*
 * Gestisce la fase di autenticazione lato client.
 */
int client_authenticate(int sock) {

    char buffer[MAX_MSG_LEN];
    char user[MAX_USERNAME_LEN];
    char pass[MAX_PASSWORD_LEN];
    char choice[10];

    while (1) {
        if (!read_input("\n1. Login\n2. Registrazione\n3. Esci\n> ", choice, sizeof(choice)))
            return -1;

        if (choice[0] == '3')
            return -1;

        if (choice[0] != '1' && choice[0] != '2') {
            printf("Scelta non valida.\n");
            continue;
        }

        if (!read_input("Username: ", user, sizeof(user))) return -1;
        if (!read_input("Password: ", pass, sizeof(pass))) return -1;

        if (strlen(user) == 0 || strlen(pass) == 0) {
            printf("Username e password non possono essere vuoti.\n");
            continue;
        }

        /* Uso le macro CMD_ per costruire il messaggio. */
        int written;
        if (choice[0] == '1')
            written = snprintf(buffer, sizeof(buffer), "%s|%s|%s\n", CMD_LOGIN, user, pass);
        else
            written = snprintf(buffer, sizeof(buffer), "%s|%s|%s\n", CMD_REGISTER, user, pass);

        if (written < 0 || written >= (int)sizeof(buffer)) {
            fprintf(stderr, "Errore: credenziali troppo lunghe.\n");
            continue;
        }

        if (send_all(sock, buffer, strlen(buffer)) < 0) {
            fprintf(stderr, "Errore di rete durante l'invio.\n");
            return -1;
        }

        int r = recv_line(sock, buffer, sizeof(buffer));
        if (r <= 0) {
            fprintf(stderr, "Connessione chiusa o timeout dal server.\n");
            return -1;
        }

        /* 
         * Poiché recv_line rimuove il '\n', ma in common.h RESP_OK ha il '\n',
         * Uso strncmp o creiamo macro "pulite" in common.h.
         */

        if (MATCH_RESP(buffer, RESP_OK)) { 
            printf("Login effettuato con successo!\n");
            return 0;
        }
        
        if (MATCH_RESP(buffer, RESP_OK_REG)) {
            printf("Registrazione completata! Ora puoi effettuare il login.\n");
            continue;
        }

        if (MATCH_RESP(buffer, RESP_TOO_MANY)) {
            printf("Troppi tentativi falliti. Disconnessione.\n");
            return -1;
        }

        /* Qualsiasi altro messaggio è un errore o notifica */
        printf("Server: %s\n", buffer);
    }
}