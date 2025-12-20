#include "users.h"
#include "server.h"

#include <stdio.h>
#include <string.h>


unsigned long hash_password(const char *password) {
    unsigned long hash = 5381;
    int c;

    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}


int authenticate(const char *username, const char *password) {
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(USER_FILE, "r");
    if (!file) { pthread_mutex_unlock(&file_mutex); return 0; }
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char stored_user[50];
        unsigned long stored_hash;

        unsigned long input_hash = hash_password(password);
        if (sscanf(line, "%49[^:]:%lu", stored_user, &stored_hash) == 2) {
            if (strcmp(username, stored_user) == 0 && input_hash == stored_hash) {
                fclose(file);
                pthread_mutex_unlock(&file_mutex);
                return 1;
            }
        }
    }
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return 0;
}

int user_exists(const char *username) {
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(USER_FILE, "r");
    if (!file) { pthread_mutex_unlock(&file_mutex); return 0; }
    char line[100], user[50];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%49[^:]", user) == 1) {
            if (strcmp(user, username) == 0) {
                fclose(file);
                pthread_mutex_unlock(&file_mutex);
                return 1;
            }
        }
    }
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return 0;
}

int create_user(const char *username, const char *password) {
    if (user_exists(username)) return 0;
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(USER_FILE, "a");
    if (!file) { pthread_mutex_unlock(&file_mutex); return 0; }
    unsigned long hash = hash_password(password);
    fprintf(file, "%s:%lu\n", username, hash);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    return 1;
}
