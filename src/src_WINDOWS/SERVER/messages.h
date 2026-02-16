#ifndef MESSAGES_H
#define MESSAGES_H

#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Carica i messaggi persistenti dal file nella cache in memoria */
int load_messages_from_file(void);

/* Salva su file lo stato corrente della cache dei messaggi */
void save_messages_to_file(void);

/* Aggiunge un nuovo messaggio alla cache del server */
void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body);

/* Invia al client tutti i messaggi destinati all'utente */
void read_messages(const char *user, socket_t client_socket);

/* Cancella tutti i messaggi destinati all'utente */
void delete_messages(const char *user);

/* Cancella un messaggio specifico dato ID */
int delete_specific_message(const char *user, int target_id);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGES_H */
