#include "messages.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

/* ID univoco progressivo per i messaggi */
static int next_message_id = 1;


/*  --------------------------------------------------------- */
/* Salva tutti i messaggi sul file (sovrascrive)             */
/*  --------------------------------------------------------- */
void save_messages_to_file() {
    pthread_mutex_lock(&file_mutex);
    pthread_mutex_lock(&cache_mutex);

    FILE *file = fopen(MSG_FILE, "w");
    if (file) {
        for (size_t i = 0; i < global_cache.count; i++) {
            Message *m = &global_cache.array[i];
            /* formato: id|recipient|sender|subject|body */
            fprintf(file, "%d|%s|%s|%s|%s\n",
                    m->id,
                    m->recipient,
                    m->sender,
                    m->subject,
                    m->body);
        }
        fclose(file);
    } else {
        perror("[ERROR] Impossibile salvare messages.txt");
    }

    pthread_mutex_unlock(&cache_mutex);
    pthread_mutex_unlock(&file_mutex);
}

/*  --------------------------------------------------------- */
/* Carica i messaggi dal file nella cache                    */
/*  --------------------------------------------------------- */
int load_messages_from_file() {
    pthread_mutex_lock(&file_mutex);

    FILE *file = fopen(MSG_FILE, "r");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    char line[MAX_MSG_LEN];
    int loaded_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        char line_copy[MAX_MSG_LEN];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        Message new_msg;

        /* parsing con ID */
        char *p_id  = strtok(line_copy, "|");
        char *p_rec = strtok(NULL, "|");
        char *p_snd = strtok(NULL, "|");
        char *p_sub = strtok(NULL, "|");
        char *p_body = strtok(NULL, "");

        if (p_id && p_rec && p_snd && p_sub && p_body) {

            new_msg.id = atoi(p_id);

            snprintf(new_msg.recipient, sizeof(new_msg.recipient), "%s", p_rec);
            snprintf(new_msg.sender, sizeof(new_msg.sender), "%s", p_snd);
            snprintf(new_msg.subject, sizeof(new_msg.subject), "%s", p_sub);
            snprintf(new_msg.body, sizeof(new_msg.body), "%s", p_body);

            pthread_mutex_lock(&cache_mutex);

            if (global_cache.count >= global_cache.capacity) {
                size_t new_capacity = (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;
                Message *new_array = realloc(global_cache.array, new_capacity * sizeof(Message));
                if (!new_array) {
                    perror("realloc fallita durante il load");
                    pthread_mutex_unlock(&cache_mutex);
                    fclose(file);
                    pthread_mutex_unlock(&file_mutex);
                    return -1;
                }
                global_cache.array = new_array;
                global_cache.capacity = new_capacity;
            }

            global_cache.array[global_cache.count++] = new_msg;

            /* aggiorna next_message_id per evitare duplicati */
            if (new_msg.id >= next_message_id) {
                next_message_id = new_msg.id + 1;
            }

            pthread_mutex_unlock(&cache_mutex);
            loaded_count++;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return loaded_count;
}

/*  --------------------------------------------------------- */
/* Salva un messaggio nella cache in RAM (thread-safe)       */
/*  --------------------------------------------------------- */
void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body) {

    pthread_mutex_lock(&cache_mutex);

    if (global_cache.count >= global_cache.capacity) {
        size_t new_capacity = (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;
        Message *new_array = realloc(global_cache.array, new_capacity * sizeof(Message));
        if (!new_array) {
            perror("realloc fallita in save_message");
            pthread_mutex_unlock(&cache_mutex);
            return;
        }
        global_cache.array = new_array;
        global_cache.capacity = new_capacity;
    }

    Message *m = &global_cache.array[global_cache.count];

    /* assegna ID univoco */
    m->id = next_message_id++;

    snprintf(m->recipient, sizeof(m->recipient), "%s", recipient);
    snprintf(m->sender, sizeof(m->sender), "%s", sender);
    snprintf(m->subject, sizeof(m->subject), "%s", subject);
    snprintf(m->body, sizeof(m->body), "%s", body);

    global_cache.count++;
    pthread_mutex_unlock(&cache_mutex);
}

/*  --------------------------------------------------------- */
/* Legge tutti i messaggi di un utente                       */
/*  --------------------------------------------------------- */
void read_messages(const char *user, int client_socket) {
    pthread_mutex_lock(&cache_mutex);
    int found = 0;

    for (size_t i = 0; i < global_cache.count; i++) {
        Message *m = &global_cache.array[i];

        if (strcmp(user, m->recipient) == 0) {
            char out[MAX_MSG_LEN];

            /* includiamo ID per permettere delete mirato */
            int n = snprintf(out, sizeof(out),
                             "FROM|%d|%s|%s|%s\n",
                             m->id,
                             m->sender,
                             m->subject,
                             m->body);

            if (n > 0)
                send(client_socket, out, (size_t)n, 0);

            found = 1;
        }
    }

    pthread_mutex_unlock(&cache_mutex);

    if (found == 0) {
        char *msg = "Nessun nuovo messaggio.\n";
        send(client_socket, msg, strlen(msg), 0);
    }

    send(client_socket, "END_READ\n", 9, 0);
}

/*  --------------------------------------------------------- */
/* Cancella tutti i messaggi di un utente                    */
/*  --------------------------------------------------------- */
void delete_messages(const char *user) {
    pthread_mutex_lock(&cache_mutex);

    size_t write_idx = 0;

    for (size_t read_idx = 0; read_idx < global_cache.count; read_idx++) {
        Message *m = &global_cache.array[read_idx];

        if (strcmp(user, m->recipient) != 0) {
            if (write_idx != read_idx) {
                global_cache.array[write_idx] = global_cache.array[read_idx];
            }
            write_idx++;
        }
    }

    global_cache.count = write_idx;
    pthread_mutex_unlock(&cache_mutex);
}

/* --------------------------------------------------------- */
/* NUOVA FUNZIONE: cancella un messaggio specifico           */
/* --------------------------------------------------------- */
int delete_specific_message(const char *user, int target_id) {

    pthread_mutex_lock(&cache_mutex);

    size_t write_idx = 0;
    int deleted = 0;

    for (size_t read_idx = 0; read_idx < global_cache.count; read_idx++) {
        Message *m = &global_cache.array[read_idx];

        if (strcmp(user, m->recipient) == 0 &&
            m->id == target_id) {

            deleted = 1;  /* non copiare → elimina */
            continue;
        }

        if (write_idx != read_idx) {
            global_cache.array[write_idx] = global_cache.array[read_idx];
        }

        write_idx++;
    }

    global_cache.count = write_idx;

    pthread_mutex_unlock(&cache_mutex);
    return deleted;
}
