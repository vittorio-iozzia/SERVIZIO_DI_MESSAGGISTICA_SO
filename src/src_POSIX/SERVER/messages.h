#ifndef MESSAGES_H
#define MESSAGES_H

#include "server.h"

/* Carica i messaggi persistenti dal file in memoria */
int load_messages_from_file(void);

/* Salva su file lo stato corrente dei messaggi */
void save_messages_to_file(void);

/* Memorizza un nuovo messaggio nella cache del server */
void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body);

/* Invia al client tutti i messaggi indirizzati all'utente */
void read_messages(const char *user, int client_socket);

/* Elimina tutti i messaggi indirizzati all'utente */
void delete_messages(const char *user);

#endif /* MESSAGES_H */
