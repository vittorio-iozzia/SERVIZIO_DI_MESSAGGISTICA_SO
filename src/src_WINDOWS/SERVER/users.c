#include "users.h"
#include "server.h"

#include <stdio.h>
#include <string.h>

#include <windows.h>


/**
 * @brief Calcola l’hash di una password tramite algoritmo DJB2.
 *
 * @param password Password in chiaro.
 * @return unsigned long Valore hash calcolato.
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
 * @brief Autentica un utente confrontando username e password.
 *
 * La password viene hashata e confrontata con il valore
 * memorizzato nel file degli utenti.
 *
 * @param username Nome utente.
 * @param password Password in chiaro.
 * @return int 1 se autenticazione riuscita, 0 altrimenti.
 */
int authenticate(const char *username, const char *password) {

    EnterCriticalSection(&file_mutex);

    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        LeaveCriticalSection(&file_mutex);
        return 0;
    }

    char line[100];
    unsigned long input_hash = hash_password(password);

    while (fgets(line, sizeof(line), file)) {

        char stored_user[50];
        unsigned long stored_hash;

        if (sscanf(line, "%49[^:]:%lu", stored_user, &stored_hash) == 2) {
            if (strcmp(username, stored_user) == 0 &&
                input_hash == stored_hash) {

                fclose(file);
                LeaveCriticalSection(&file_mutex);
                return 1;
            }
        }
    }

    fclose(file);
    LeaveCriticalSection(&file_mutex);
    return 0;
}


/**
 * @brief Verifica se un utente è già registrato.
 *
 * @param username Nome utente da verificare.
 * @return int 1 se l’utente esiste, 0 altrimenti.
 */
int user_exists(const char *username) {

    EnterCriticalSection(&file_mutex);

    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        LeaveCriticalSection(&file_mutex);
        return 0;
    }

    char line[100];
    char user[50];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%49[^:]", user) == 1) {
            if (strcmp(user, username) == 0) {
                fclose(file);
                LeaveCriticalSection(&file_mutex);
                return 1;
            }
        }
    }

    fclose(file);
    LeaveCriticalSection(&file_mutex);
    return 0;
}


/**
 * @brief Registra un nuovo utente nel sistema.
 *
 * L’utente viene aggiunto al file persistente solo se
 * non esiste già.
 *
 * @param username Nome utente.
 * @param password Password in chiaro.
 * @return int 1 se la registrazione è avvenuta con successo, 0 altrimenti.
 */
int create_user(const char *username, const char *password) {

    if (user_exists(username))
        return 0;

    EnterCriticalSection(&file_mutex);

    FILE *file = fopen(USER_FILE, "a");
    if (!file) {
        LeaveCriticalSection(&file_mutex);
        return 0;
    }

    unsigned long hash = hash_password(password);
    fprintf(file, "%s:%lu\n", username, hash);

    fclose(file);
    LeaveCriticalSection(&file_mutex);
    return 1;
}
