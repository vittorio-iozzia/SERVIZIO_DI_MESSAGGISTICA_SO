#ifndef AUTH_H
#define AUTH_H

#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Avvia la procedura di autenticazione lato client.
 *
 * Gestisce login e registrazione comunicando con il server
 * tramite protocollo testuale su socket TCP.
 */
int client_authenticate(socket_t sock);

#ifdef __cplusplus
}
#endif

#endif /* AUTH_H */
