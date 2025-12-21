#ifndef MESSAGES_H
#define MESSAGES_H

#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

int load_messages_from_file(void);
void save_messages_to_file(void);

void save_message(const char *sender,
                  const char *recipient,
                  const char *subject,
                  const char *body);

void read_messages(const char *user, socket_t client_socket);
void delete_messages(const char *user);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGES_H */
