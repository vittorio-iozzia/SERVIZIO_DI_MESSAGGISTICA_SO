#ifndef MESSAGES_H
#define MESSAGES_H

#include "server.h"

int load_messages_from_file();
void save_messages_to_file();
void save_message(const char *sender, const char *recipient,
                  const char *subject, const char *body);
void read_messages(const char *user, int client_socket);
void delete_messages(const char *user);

#endif
