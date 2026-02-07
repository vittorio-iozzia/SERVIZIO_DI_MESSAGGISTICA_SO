#ifndef USERS_H
#define USERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Verifica le credenziali di un utente */
int authenticate(const char *username, const char *password);

/* Controlla se un utente è già registrato */
int user_exists(const char *username);

/* Registra un nuovo utente nel sistema */
int create_user(const char *username, const char *password);

/* Calcola l'hash di una password */
unsigned long hash_password(const char *password);

#ifdef __cplusplus
}
#endif

#endif /* USERS_H */
