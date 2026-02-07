#ifndef USERS_H
#define USERS_H

/* Verifica le credenziali di un utente */
int authenticate(const char *username, const char *password);

/* Controlla se un utente è registrato */
int user_exists(const char *username);

/* Registra un nuovo utente nel sistema */
int create_user(const char *username, const char *password);

/* Calcola l'hash della password */
unsigned long hash_password(const char *password);

#endif /* USERS_H */
