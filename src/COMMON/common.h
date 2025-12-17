#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/* ===========================
 *  DIMENSIONI MASSIME DATI
 * =========================== */

/* Username e password */
#define MAX_USERNAME_LEN   50
#define MAX_PASSWORD_LEN   50

/* Campi del messaggio */
#define MAX_SUBJECT_LEN    100
#define MAX_BODY_LEN       1024

/* Dimensione massima di una riga di protocollo */
#define MAX_MSG_LEN        2048


/* ===========================
 *  DELIMITATORI PROTOCOLLO
 * =========================== */

#define PROTO_FIELD_DELIM  '|'
#define PROTO_LINE_END     '\n'


/* ===========================
 *  COMANDI PROTOCOLLO
 * =========================== */

/* Client -> Server */
#define CMD_LOGIN      "LOGIN"
#define CMD_REGISTER   "REGISTER"
#define CMD_SEND       "SEND"
#define CMD_READ       "READ"
#define CMD_DELETE     "DELETE"
#define CMD_QUIT       "QUIT"

/* Server -> Client */
#define RESP_OK            "OK"
#define RESP_OK_REG        "OK_REG"
#define RESP_FAIL          "FAIL"
#define RESP_FAIL_EXISTS   "FAIL_EXISTS"
#define RESP_ERR           "ERR"
#define RESP_BYE           "BYE"
#define RESP_END_READ      "END_READ"


/* ===========================
 *  NOTE
 * =========================== */
/*
 * Questo file è condiviso tra client e server
 * per garantire coerenza nelle dimensioni dei buffer
 * e nelle stringhe del protocollo testuale.
 */

#endif
