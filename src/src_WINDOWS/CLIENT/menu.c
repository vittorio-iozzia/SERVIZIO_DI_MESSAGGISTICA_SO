#include "menu.h"
#include "net_utils.h"
#include "input_utils.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>


/**
 * @brief Gestisce il menu interattivo del client.
 *
 * La funzione mostra un menu testuale che permette all'utente
 * di inviare messaggi, leggere la posta, cancellare i messaggi
 * o terminare la sessione.
 *
 * @param sock Socket connesso al server.
 */
void client_menu_loop(socket_t sock) {

    char buffer[MAX_MSG_LEN];

    while (1) {

        printf("\n=== MENU ===\n");
        printf("1. Invia messaggio\n");
        printf("2. Leggi messaggi\n");
        printf("3. Cancella messaggi\n");
        printf("4. Esci\n> ");

        char choice_input[10];
        if (!fgets(choice_input, sizeof(choice_input), stdin))
            return;

        int choice = atoi(choice_input);

        if (choice == 1) {

            char to[MAX_USERNAME_LEN];
            char subj[MAX_SUBJECT_LEN];
            char body[MAX_BODY_LEN];

            printf("A: ");
            if (!fgets(to, sizeof(to), stdin))
                continue;
            clean_input(to);

            printf("Oggetto: ");
            if (!fgets(subj, sizeof(subj), stdin))
                continue;
            clean_input(subj);

            printf("Testo: ");
            if (!fgets(body, sizeof(body), stdin))
                continue;
            clean_input(body);

            snprintf(buffer, sizeof(buffer),
                     "SEND|%s|%s|%s\n", to, subj, body);

            send(sock, buffer, (int)strlen(buffer), 0);

            if (recv_line(sock, buffer, sizeof(buffer)) > 0)
                printf("Server: %s\n", buffer);

        } else if (choice == 2) {

            send(sock, "READ\n", 5, 0);
            printf("\n--- Posta in arrivo ---\n");

            while (1) {

                if (recv_line(sock, buffer, sizeof(buffer)) <= 0)
                    return;

                if (strcmp(buffer, "END_READ") == 0)
                    break;

                printf("%s\n", buffer);
            }

        } else if (choice == 3) {

            send(sock, "DELETE\n", 7, 0);

            if (recv_line(sock, buffer, sizeof(buffer)) > 0)
                printf("Server: %s\n", buffer);

        } else if (choice == 4) {

            send(sock, "QUIT\n", 5, 0);
            return;

        } else {
            printf("Scelta non valida.\n");
        }
    }
}
