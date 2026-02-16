#ifndef MESSAGES_H
#define MESSAGES_H

#include "server.h"


/* ----------------------------------------------------- */
/* GESTIONE PERSISTENZA MESSAGGI                         */
/* ----------------------------------------------------- */

/* Carica i messaggi persistenti dal file in memoria */
int load_messages_from_file(void);

/* Salva su file lo stato corrente dei messaggi */
void save_messages_to_file(void);

/* ----------------------------------------------------- */
/* GESTIONE CACHE MESSAGGI                               */
/* ----------------------------------------------------- */

/* Memorizza un nuovo messaggio nella cache del server */
void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body);

/* Invia al client tutti i messaggi indirizzati all'utente */
void read_messages(const char *user, int client_socket);

/* Elimina TUTTI i messaggi indirizzati all'utente */
void delete_messages(const char *user);


/* Elimina UN messaggio specifico di un utente (tramite ID)
 * Ritorna 1 se eliminato, 0 se non trovato
 */
int delete_specific_message(const char *user, int message_id);

#endif /* MESSAGES_H */
