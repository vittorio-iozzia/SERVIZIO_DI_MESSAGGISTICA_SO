#include "users.h"
#include "server.h"
#include "../COMMON/common.h" 

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/**
 * Calcola l'hash di una password utilizzando l'algoritmo djb2.
 * Formula: hash(i) = hash(i-1) * 33 + str[i]
 */
unsigned long hash_password(const char *password) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash;
}

/**
 * Verifica le credenziali confrontando l'hash calcolato con quello memorizzato.
 */
int authenticate(const char *username, const char *password) {
    if (!username || !password || 
        strlen(username) >= MAX_USERNAME_LEN || 
        strlen(password) >= MAX_PASSWORD_LEN) {
        return 0;
    }

    pthread_mutex_lock(&file_mutex);

    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    char line[MAX_USERNAME_LEN + 64]; 
    unsigned long input_hash = hash_password(password);
    char stored_user[MAX_USERNAME_LEN];
    unsigned long stored_hash;

    // Costruzione dinamica del formato per sscanf (sicurezza contro overflow)
    char format[32];
    snprintf(format, sizeof(format), "%%%zu[^:]:%%lu", (size_t)MAX_USERNAME_LEN - 1);

    int auth_success = 0;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, format, stored_user, &stored_hash) == 2) {
            if (strcmp(username, stored_user) == 0 && input_hash == stored_hash) {
                auth_success = 1;
                break;
            }
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return auth_success;
}

/**
 * Verifica se un username è già presente nel database.
 */
int user_exists(const char *username) {
    if (!username || strlen(username) >= MAX_USERNAME_LEN) return 0;

    pthread_mutex_lock(&file_mutex);
    
    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    char line[MAX_USERNAME_LEN + 64];
    char user[MAX_USERNAME_LEN];
    char format[32];
    snprintf(format, sizeof(format), "%%%zu[^:]", (size_t)MAX_USERNAME_LEN - 1);

    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, format, user) == 1) {
            if (strcmp(user, username) == 0) {
                found = 1;
                break;
            }
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return found;
}

/**
 * Registra un nuovo utente. Implementa la persistenza atomica tramite rename().
 */
int create_user(const char *username, const char *password) {
    if (!username || !password || 
        strlen(username) == 0 || strlen(password) == 0 ||
        strlen(username) >= MAX_USERNAME_LEN || 
        strlen(password) >= MAX_PASSWORD_LEN) {
        return 0;
    }

    pthread_mutex_lock(&file_mutex);

    const char *tmp_user_file = "DATA/users.tmp";
    FILE *rf = fopen(USER_FILE, "r");
    FILE *wf = fopen(tmp_user_file, "w");

    if (!wf) {
        if (rf) fclose(rf);
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    char line[MAX_USERNAME_LEN + 64];
    char user[MAX_USERNAME_LEN];
    char format[32];
    snprintf(format, sizeof(format), "%%%zu[^:]", (size_t)MAX_USERNAME_LEN - 1);

    int exists = 0;
    if (rf) {
        while (fgets(line, sizeof(line), rf)) {
            if (sscanf(line, format, user) == 1) {
                if (strcmp(user, username) == 0) exists = 1;
            }
            fputs(line, wf); 
        }
        fclose(rf);
    }

    if (exists) {
        fclose(wf);
        unlink(tmp_user_file);
        pthread_mutex_unlock(&file_mutex);
        return 0; 
    }

    // Aggiunta nuovo utente
    unsigned long hash = hash_password(password);
    if (fprintf(wf, "%s:%lu\n", username, hash) < 0) {
        fclose(wf);
        unlink(tmp_user_file);
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    fflush(wf);
    fsync(fileno(wf));
    fclose(wf);

    // Swap atomico
    if (rename(tmp_user_file, USER_FILE) != 0) {
        unlink(tmp_user_file);
        pthread_mutex_unlock(&file_mutex);
        return 0;
    }

    pthread_mutex_unlock(&file_mutex);
    return 1;
}