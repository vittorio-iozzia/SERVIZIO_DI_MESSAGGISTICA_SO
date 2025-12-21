#ifndef USERS_H
#define USERS_H

#ifdef __cplusplus
extern "C" {
#endif

int authenticate(const char *username, const char *password);
int user_exists(const char *username);
int create_user(const char *username, const char *password);

unsigned long hash_password(const char *password);

#ifdef __cplusplus
}
#endif

#endif /* USERS_H */
