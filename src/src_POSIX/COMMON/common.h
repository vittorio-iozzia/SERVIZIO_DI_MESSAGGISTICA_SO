#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/* Dimensioni massime di username e password */
#define MAX_USERNAME_LEN   50
#define MAX_PASSWORD_LEN   50

/* Dimensioni massime dei campi di un messaggio */
#define MAX_SUBJECT_LEN    100
#define MAX_BODY_LEN       1024

/* Dimensione massima di una riga del protocollo */
#define MAX_MSG_LEN        2048

/* Delimitatori del protocollo testuale */
#define PROTO_FIELD_DELIM  '|'
#define PROTO_LINE_END     '\n'

/* Comandi inviati dal client al server */
#define CMD_LOGIN      "LOGIN"
#define CMD_REGISTER   "REGISTER"
#define CMD_SEND       "SEND"
#define CMD_READ       "READ"
#define CMD_DELETE     "DELETE"
#define CMD_QUIT       "QUIT"

/* Risposte inviate dal server al client */
#define RESP_OK            "OK"
#define RESP_OK_REG        "OK_REG"
#define RESP_FAIL          "FAIL"
#define RESP_FAIL_EXISTS   "FAIL_EXISTS"
#define RESP_ERR           "ERR"
#define RESP_BYE           "BYE"
#define RESP_END_READ      "END_READ"

/*
 * Header condiviso tra client e server.
 * Centralizza le costanti del protocollo e le dimensioni dei buffer
 * per garantire coerenza tra le due componenti.
 */

#endif /* COMMON_H */
