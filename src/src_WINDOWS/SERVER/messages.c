#include "messages.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>


/**
 * @brief Salva tutti i messaggi presenti nella cache su file.
 *
 * La funzione sovrascrive completamente il file dei messaggi
 * persistenti. L’accesso alla cache e al file è protetto da mutex
 * per garantire la consistenza dei dati in ambiente concorrente.
 */
void save_messages_to_file(void) {

    EnterCriticalSection(&file_mutex);
    EnterCriticalSection(&cache_mutex);

    FILE *file = fopen(MSG_FILE, "w");
    if (file) {
        for (size_t i = 0; i < global_cache.count; i++) {
            Message *m = &global_cache.array[i];
            fprintf(file, "%s|%s|%s|%s\n",
                    m->recipient,
                    m->sender,
                    m->subject,
                    m->body);
        }
        fclose(file);
    } else {
        perror("[ERROR] Impossibile salvare messages.txt");
    }

    LeaveCriticalSection(&cache_mutex);
    LeaveCriticalSection(&file_mutex);
}


/**
 * @brief Carica i messaggi persistenti dal file nella cache in memoria.
 *
 * Ogni riga del file viene parsata e convertita in una struttura
 * Message. La cache viene ridimensionata dinamicamente se necessario.
 *
 * @return int Numero di messaggi caricati correttamente.
 */
int load_messages_from_file(void) {

    EnterCriticalSection(&file_mutex);

    FILE *file = fopen(MSG_FILE, "r");
    if (!file) {
        LeaveCriticalSection(&file_mutex);
        return 0;
    }

    char line[MAX_MSG_LEN];
    int loaded_count = 0;

    while (fgets(line, sizeof(line), file)) {

        line[strcspn(line, "\n")] = '\0';

        char line_copy[MAX_MSG_LEN];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        Message new_msg;

        char *p_rec  = strtok(line_copy, "|");
        char *p_snd  = strtok(NULL, "|");
        char *p_sub  = strtok(NULL, "|");
        char *p_body = strtok(NULL, "");

        if (p_rec && p_snd && p_sub && p_body) {

            snprintf(new_msg.recipient, sizeof(new_msg.recipient), "%s", p_rec);
            snprintf(new_msg.sender,    sizeof(new_msg.sender),    "%s", p_snd);
            snprintf(new_msg.subject,   sizeof(new_msg.subject),   "%s", p_sub);
            snprintf(new_msg.body,      sizeof(new_msg.body),      "%s", p_body);

            EnterCriticalSection(&cache_mutex);

            if (global_cache.count >= global_cache.capacity) {

                size_t new_capacity =
                    (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;

                Message *new_array =
                    realloc(global_cache.array, new_capacity * sizeof(Message));

                if (!new_array) {
                    perror("realloc fallita durante il load");
                    LeaveCriticalSection(&cache_mutex);
                    fclose(file);
                    LeaveCriticalSection(&file_mutex);
                    return -1;
                }

                global_cache.array    = new_array;
                global_cache.capacity = new_capacity;
            }

            global_cache.array[global_cache.count++] = new_msg;

            LeaveCriticalSection(&cache_mutex);
            loaded_count++;
        }
    }

    fclose(file);
    LeaveCriticalSection(&file_mutex);
    return loaded_count;
}


/**
 * @brief Salva un nuovo messaggio nella cache in memoria.
 *
 * Il messaggio viene aggiunto alla cache condivisa. Se necessario,
 * la struttura dati viene ridimensionata dinamicamente.
 *
 * @param sender Mittente del messaggio.
 * @param recipient Destinatario del messaggio.
 * @param subject Oggetto del messaggio.
 * @param body Corpo del messaggio.
 */
void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body) {

    EnterCriticalSection(&cache_mutex);

    if (global_cache.count >= global_cache.capacity) {

        size_t new_capacity =
            (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;

        Message *new_array =
            realloc(global_cache.array, new_capacity * sizeof(Message));

        if (!new_array) {
            perror("realloc fallita in save_message");
            LeaveCriticalSection(&cache_mutex);
            return;
        }

        global_cache.array    = new_array;
        global_cache.capacity = new_capacity;
    }

    Message *m = &global_cache.array[global_cache.count];

    snprintf(m->recipient, sizeof(m->recipient), "%s", recipient);
    snprintf(m->sender,    sizeof(m->sender),    "%s", sender);
    snprintf(m->subject,   sizeof(m->subject),   "%s", subject);
    snprintf(m->body,      sizeof(m->body),      "%s", body);

    global_cache.count++;
    LeaveCriticalSection(&cache_mutex);
}


/**
 * @brief Invia al client tutti i messaggi destinati a un utente.
 *
 * I messaggi vengono inviati uno alla volta tramite il socket
 * associato al client. Al termine viene inviato un messaggio
 * di fine lettura.
 *
 * @param user Username del destinatario.
 * @param client_socket Socket del client connesso.
 */
void read_messages(const char *user, socket_t client_socket) {

    EnterCriticalSection(&cache_mutex);
    int found = 0;

    for (size_t i = 0; i < global_cache.count; i++) {

        Message *m = &global_cache.array[i];

        if (strcmp(user, m->recipient) == 0) {

            char out[MAX_MSG_LEN];
            int n = snprintf(out, sizeof(out),
                             "FROM|%s|%s|%s\n",
                             m->sender,
                             m->subject,
                             m->body);

            if (n > 0) {
                send(client_socket, out, (int)strlen(out), 0);
            }
            found = 1;
        }
    }

    LeaveCriticalSection(&cache_mutex);

    if (!found) {
        const char *msg = "Nessun nuovo messaggio.\n";
        send(client_socket, msg, (int)strlen(msg), 0);
    }

    send(client_socket, "END_READ\n", 9, 0);
}


/**
 * @brief Cancella tutti i messaggi destinati a un utente.
 *
 * I messaggi non destinati all’utente vengono compattati
 * nella cache, mentre quelli corrispondenti vengono rimossi.
 *
 * @param user Username dell’utente.
 */
void delete_messages(const char *user) {

    EnterCriticalSection(&cache_mutex);

    size_t write_idx = 0;

    for (size_t read_idx = 0; read_idx < global_cache.count; read_idx++) {

        Message *m = &global_cache.array[read_idx];

        if (strcmp(user, m->recipient) != 0) {
            if (write_idx != read_idx) {
                global_cache.array[write_idx] =
                    global_cache.array[read_idx];
            }
            write_idx++;
        }
    }

    global_cache.count = write_idx;
    LeaveCriticalSection(&cache_mutex);
}
