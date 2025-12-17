#ifndef USERS_H
#define USERS_H

int authenticate(const char *username, const char *password);
int user_exists(const char *username);
int create_user(const char *username, const char *password);

#endif
