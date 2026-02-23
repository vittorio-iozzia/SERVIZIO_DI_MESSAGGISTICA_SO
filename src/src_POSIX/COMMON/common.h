#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/* --- CONFIGURAZIONE SISTEMA --- */
#define PORT               8080
#define MAX_CLIENTS        100      /* [DoS] Limite massimo di connessioni simultanee */
#define USER_FILE          "DATA/users.txt"
#define MSG_FILE           "DATA/messages.txt"

/* --- DIMENSIONI BUFFER (COERENTI CON I FIX) --- */
#define MAX_USERNAME_LEN   50
#define MAX_PASSWORD_LEN   50
#define MAX_SUBJECT_LEN    100
#define MAX_BODY_LEN       1024
#define MAX_MSG_LEN        2048    /* Spazio sufficiente per comando + dati + separatori */
#define MAX_SERVER_CACHE   100000

/* --- DELIMITATORI PROTOCOLLO --- */
#define PROTO_FIELD_DELIM  '|'
#define PROTO_LINE_END     '\n'

/* --- COMANDI CLIENT (Senza newline, usati con strtok_r) --- */
#define CMD_LOGIN          "LOGIN"
#define CMD_REGISTER       "REGISTER"
#define CMD_SEND           "SEND"
#define CMD_READ           "READ"
#define CMD_DELETE         "DELETE"
#define CMD_DELETE_ONE     "DELETE_ONE" 
#define CMD_QUIT           "QUIT"

/* --- RISPOSTE SERVER (Con newline per recv_line) --- */
/* Includere il \n qui rende il codice del server molto più pulito e meno propenso a errori */
#define RESP_OK            "OK\n"
#define RESP_OK_REG        "OK_REG\n"
#define RESP_FAIL          "FAIL\n"
#define RESP_FAIL_EXISTS   "FAIL_EXISTS\n"
#define RESP_ERR           "ERR\n"
#define RESP_BYE           "BYE\n"

/* --- NOTIFICHE AVANZATE --- */
#define RESP_COUNT         "COUNT"             /* Usato con snprintf per aggiungere il numero */
#define RESP_TOO_MANY      "TOO_MANY_ATTEMPTS\n"
#define RESP_ERR_NO_USER   "ERR_NO_USER\n"
#define RESP_ERR_NOT_FOUND "ERR_NOT_FOUND\n"
#define RESP_ERR_FORMAT    "ERR_INVALID_FORMAT\n"

#endif /* COMMON_H */