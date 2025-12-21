#ifndef AUTH_H
#define AUTH_H

#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

int client_authenticate(socket_t sock);

#ifdef __cplusplus
}
#endif

#endif /* AUTH_H */
