#include "messages.h"
#include "server.h"
#include "../COMMON/net_utils.h" 
#include "../COMMON/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>

static int next_message_id = 1;

/* [OTTIMIZZAZIONE] "Dirty Bit"
 * Serve a evitare inutili e costose scritture sul disco (I/O). 
 * Vale 1 se la RAM è stata modificata, 0 se è già sincronizzata col disco. */
static int cache_modified = 0; 

/* UTILITY: SANITIZE INPUT */
void sanitize_input(char *str, size_t max_len) {
    for (size_t i = 0; i < max_len && str[i]; i++) {
        if (str[i] == '|' || str[i] == '\n' || str[i] == '\r') {
            str[i] = '_';
        }
    }
}

/* RISCRITTURA COMPLETA ATOMICA DEL FILE */
void save_messages_to_file() {
    const char *tmp_file = "DATA/messages.tmp";

    pthread_mutex_lock(&cache_mutex);
    
    /* [OTTIMIZZAZIONE I/O] Se nessuno ha modificato i messaggi, esco subito */
    if (!cache_modified) {
        pthread_mutex_unlock(&cache_mutex);
        return;
    }

    size_t count = global_cache.count;
    Message *copy = NULL;
    
    if (count > 0) {
        copy = malloc(count * sizeof(Message));
        if (!copy) {
            perror("[ERROR] malloc fallita");
            pthread_mutex_unlock(&cache_mutex);
            return;
        }
        memcpy(copy, global_cache.array, count * sizeof(Message));
    }
    
    cache_modified = 0;
    pthread_mutex_unlock(&cache_mutex);

    /* Se non ci sono messaggi, svuotiamo semplicemente il file */
    if (count == 0) {
        pthread_mutex_lock(&file_mutex);
        FILE *f = fopen(MSG_FILE, "w");
        if (f) fclose(f);
        pthread_mutex_unlock(&file_mutex);
        free(copy);
        return;
    }

    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(tmp_file, "w");
    if (!file) {
        perror("[ERROR] Impossibile aprire tmp file");
        pthread_mutex_unlock(&file_mutex);
        free(copy);
        return;
    }

    int write_error = 0;
    for (size_t i = 0; i < count; i++) {
        if (fprintf(file, "%d|%s|%s|%s|%s\n",
                    copy[i].id, copy[i].recipient,
                    copy[i].sender, copy[i].subject,
                    copy[i].body) < 0) {
            write_error = 1;
            break;
        }
    }

    if (write_error) {
        fclose(file);
        unlink(tmp_file); 
    } else {
        fflush(file);
        fsync(fileno(file));
        fclose(file);
        if (rename(tmp_file, MSG_FILE) != 0) {
            unlink(tmp_file);
        }
    }

    pthread_mutex_unlock(&file_mutex);
    free(copy);
}

/* CARICAMENTO INIZIALE DEI MESSAGGI */
int load_messages_from_file() {
    FILE *file = fopen(MSG_FILE, "r");
    if (!file) return 0;

    char line[MAX_MSG_LEN];
    char *saveptr;
    int loaded = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char *p_id  = strtok_r(line, "|", &saveptr);
        char *p_rec = strtok_r(NULL, "|", &saveptr);
        char *p_snd = strtok_r(NULL, "|", &saveptr);
        char *p_sub = strtok_r(NULL, "|", &saveptr);
        char *p_body = strtok_r(NULL, "", &saveptr);

        if (!p_id || !p_rec || !p_snd || !p_sub || !p_body) continue;

        /* Sostituito atoi con strtol per parsing ID robusto */
        char *endptr;
        long id_val = strtol(p_id, &endptr, 10);
        if (endptr == p_id) continue;

        pthread_mutex_lock(&cache_mutex);
        
        /* [OOM] Interrompo il caricamento se supero il limite di sicurezza definito in common.h */
        if (global_cache.count >= MAX_SERVER_CACHE) {
            fprintf(stderr, "[WARNING] Raggiunto limite massimo cache messaggi durante il caricamento.\n");
            pthread_mutex_unlock(&cache_mutex);
            break;
        }

        if (global_cache.count >= global_cache.capacity) {
            size_t new_cap = (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;
            Message *new_arr = realloc(global_cache.array, new_cap * sizeof(Message));
            if (!new_arr) { 
                pthread_mutex_unlock(&cache_mutex); 
                break; 
            }
            global_cache.array = new_arr;
            global_cache.capacity = new_cap;
        }

        Message *m = &global_cache.array[global_cache.count++];
        m->id = (int)id_val;
        strncpy(m->recipient, p_rec, MAX_USERNAME_LEN-1);
        m->recipient[MAX_USERNAME_LEN-1] = '\0';
        strncpy(m->sender, p_snd, MAX_USERNAME_LEN-1);
        m->sender[MAX_USERNAME_LEN-1] = '\0';
        strncpy(m->subject, p_sub, MAX_SUBJECT_LEN-1);
        m->subject[MAX_SUBJECT_LEN-1] = '\0';
        strncpy(m->body, p_body, MAX_BODY_LEN-1);
        m->body[MAX_BODY_LEN-1] = '\0';
        
        if (m->id >= next_message_id) next_message_id = m->id + 1;
        pthread_mutex_unlock(&cache_mutex);
        loaded++;
    }

    fclose(file);
    return loaded;
}

/* SALVATAGGIO SINGOLO MESSAGGIO */
void save_message(const char *sender, const char *recipient, const char *subject, const char *body) {
    pthread_mutex_lock(&cache_mutex);
    
    /* [OOM] Blocco l'inserimento se il server ha troppi messaggi in RAM (limite in common.h) */
    if (global_cache.count >= MAX_SERVER_CACHE) {
        fprintf(stderr, "[WARNING] Impossibile salvare il messaggio: cache piena!\n");
        pthread_mutex_unlock(&cache_mutex);
        return;
    }

    if (global_cache.count >= global_cache.capacity) {
        size_t new_cap = (global_cache.capacity == 0) ? 100 : global_cache.capacity * 2;
        Message *new_arr = realloc(global_cache.array, new_cap * sizeof(Message));
        if (!new_arr) {
            pthread_mutex_unlock(&cache_mutex);
            return;
        }
        global_cache.array = new_arr;
        global_cache.capacity = new_cap;
    }

    Message *m = &global_cache.array[global_cache.count++];
    m->id = next_message_id++;
    
    strncpy(m->sender, sender, MAX_USERNAME_LEN-1);
    m->sender[MAX_USERNAME_LEN-1] = '\0';
    strncpy(m->recipient, recipient, MAX_USERNAME_LEN-1);
    m->recipient[MAX_USERNAME_LEN-1] = '\0';
    strncpy(m->subject, subject, MAX_SUBJECT_LEN-1);
    m->subject[MAX_SUBJECT_LEN-1] = '\0';
    strncpy(m->body, body, MAX_BODY_LEN-1);
    m->body[MAX_BODY_LEN-1] = '\0';
    
    sanitize_input(m->sender, MAX_USERNAME_LEN);
    sanitize_input(m->recipient, MAX_USERNAME_LEN);
    sanitize_input(m->subject, MAX_SUBJECT_LEN);
    sanitize_input(m->body, MAX_BODY_LEN);

    /* [DIRTY BIT] Abbiamo aggiunto un messaggio, la RAM è "sporca" */
    cache_modified = 1; 
    pthread_mutex_unlock(&cache_mutex);
}

/* LETTURA MESSAGGI */
void read_messages(const char *user, int client_socket) {
    Message *to_send = NULL;
    int count = 0, cap = 0;

    pthread_mutex_lock(&cache_mutex);
    for (size_t i = 0; i < global_cache.count; i++) {
        if (strcmp(user, global_cache.array[i].recipient) == 0) {
            if (count >= cap) {
                int n_cap = (cap == 0) ? 10 : cap * 2;
                Message *tmp = realloc(to_send, n_cap * sizeof(Message));
                if (!tmp) break;
                to_send = tmp; 
                cap = n_cap;
            }
            to_send[count++] = global_cache.array[i];
        }
    }
    pthread_mutex_unlock(&cache_mutex);

    char buf[MAX_MSG_LEN];

    int n = snprintf(buf, sizeof(buf), "%s|%d\n", RESP_COUNT, count);
    if (n > 0 && send_all(client_socket, buf, (size_t)n) >= 0) {
        for (int i = 0; i < count; i++) {
            n = snprintf(buf, sizeof(buf), "FROM|%d|%s|%s|%s\n", 
                             to_send[i].id, to_send[i].sender, to_send[i].subject, to_send[i].body);
            if (n <= 0 || send_all(client_socket, buf, (size_t)n) < 0) break;
        }
    }
    if (to_send) free(to_send);
}

/* CANCELLA TUTTI I MESSAGGI DI UN UTENTE */
void delete_messages(const char *user) {
    pthread_mutex_lock(&cache_mutex);
    size_t w = 0;
    for (size_t r = 0; r < global_cache.count; r++) {
        if (strcmp(user, global_cache.array[r].recipient) != 0)
            global_cache.array[w++] = global_cache.array[r];
    }
    
    if (global_cache.count != w) cache_modified = 1;
    global_cache.count = w;
    pthread_mutex_unlock(&cache_mutex);

}

/* CANCELLA MESSAGGIO SPECIFICO */
int delete_specific_message(const char *user, int target_id) {
    pthread_mutex_lock(&cache_mutex);
    size_t w = 0; int del = 0;
    for (size_t r = 0; r < global_cache.count; r++) {
        if (strcmp(user, global_cache.array[r].recipient) == 0 && global_cache.array[r].id == target_id) {
            del = 1; 
            continue;
        }
        global_cache.array[w++] = global_cache.array[r];
    }
    
    if (del) cache_modified = 1;
    global_cache.count = w;
    pthread_mutex_unlock(&cache_mutex);

    return del;
}