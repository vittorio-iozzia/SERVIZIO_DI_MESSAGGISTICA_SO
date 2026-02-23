#include "menu.h"
#include "../COMMON/net_utils.h"
#include "input_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include "../COMMON/common.h"

#define MAX_INBOX_SIZE 10000 /* Limite di sicurezza anti-DoS */

void client_menu_loop(int sock) {
    char buffer[MAX_MSG_LEN];
    char choice_input[10];

    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Invia messaggio\n");
        printf("2. Leggi messaggi\n");
        printf("3. Cancella tutti i messaggi\n");
        printf("4. Cancella un messaggio specifico\n");
        printf("5. Esci\n");

        if (!read_input("> ", choice_input, sizeof(choice_input))) return;

        char *endptr_menu;
        long choice = strtol(choice_input, &endptr_menu, 10);
        if (endptr_menu == choice_input || *endptr_menu != '\0' || choice < 1 || choice > 5) {  /* Nel primo controllo ho aggiunto una protezione contro input non numerici */
            printf("Scelta non valida. Inserisci un numero tra 1 e 5.\n");
            continue;
        }

        if (choice == 1) {  /* Invia messaggio */
            char to[MAX_USERNAME_LEN];
            char subj[MAX_SUBJECT_LEN];
            char body[MAX_BODY_LEN];

            if (!read_input("A: ", to, sizeof(to))) return;
            if (!read_input("Oggetto: ", subj, sizeof(subj))) return;
            if (!read_input("Testo: ", body, sizeof(body))) return;

            int written = snprintf(buffer, sizeof(buffer), "%s|%s|%s|%s\n", 
                                   CMD_SEND, to, subj, body);
            
            if (written < 0 || written >= (int)sizeof(buffer)) {
                fprintf(stderr, "Errore: Messaggio troppo lungo.\n");
                continue;
            }

            if (send_all(sock, buffer, (size_t)written) < 0) return;
            if (recv_line(sock, buffer, sizeof(buffer)) <= 0) return;

            printf("Server: %s\n", buffer);

        } else if (choice == 2) {  /* Leggi messaggi */
            
            snprintf(buffer, sizeof(buffer), "%s\n", CMD_READ);
            if (send_all(sock, buffer, strlen(buffer)) < 0) return;
            
            if (recv_line(sock, buffer, sizeof(buffer)) <= 0) return;

            int msg_count = 0;
            
            char format[32];
            snprintf(format, sizeof(format), "%s|%%d", RESP_COUNT);

            if (sscanf(buffer, format, &msg_count) != 1) {  /* Con != 1 mi assicuro che il client non provi a leggere un numero di messaggi causuale rimasto nella variabile msg count*/
                fprintf(stderr, "[ERRORE] Risposta imprevista: %s\n", buffer);
                continue;
            }

            if (msg_count < 0 || msg_count > MAX_INBOX_SIZE) {
                fprintf(stderr, "[ERRORE] Numero messaggi non valido (%d).\n", msg_count);
                return; 
            }

            if (msg_count == 0) {
                printf("\n--- La tua casella di posta è vuota ---\n");
                continue;
            }

            printf("\n--- Posta in arrivo (%d righe) ---\n", msg_count);
            for (int i = 0; i < msg_count; i++) {
                if (recv_line(sock, buffer, sizeof(buffer)) <= 0) return;
                printf("%s\n", buffer);
            }
            printf("--- Fine ---\n");

        } else if (choice == 3) {  /* Cancella tutti */
            snprintf(buffer, sizeof(buffer), "%s\n", CMD_DELETE);
            if (send_all(sock, buffer, strlen(buffer)) < 0) return;
            if (recv_line(sock, buffer, sizeof(buffer)) <= 0) return;
            printf("Server: %s\n", buffer);

        } else if (choice == 4) {  /* Cancella specifico */
            char id_input[16];
            if (!read_input("ID messaggio: ", id_input, sizeof(id_input))) continue;

            char *endptr;
            long val = strtol(id_input, &endptr, 10);
            if (endptr == id_input || *endptr != '\0' || val <= 0 || val > INT_MAX) {
                printf("ID non valido.\n");
                continue;
            }

            snprintf(buffer, sizeof(buffer), "%s|%ld\n", CMD_DELETE_ONE, val);
            if (send_all(sock, buffer, strlen(buffer)) < 0) return;
            if (recv_line(sock, buffer, sizeof(buffer)) <= 0) return;
            printf("Server: %s\n", buffer);

        } else if (choice == 5) {  /* Esci */
            snprintf(buffer, sizeof(buffer), "%s\n", CMD_QUIT);
            send_all(sock, buffer, strlen(buffer));
            return;
        }
    }
}