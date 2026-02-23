#include "client.h"

/* Aggiunti argomenti della riga di comando per rendere l'IP dinamico */
int main(int argc, char *argv[]) {
    /* Se l'utente non specifica l'IP, usiamo localhost come default di sicurezza */
    const char *server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    
    client_program(server_ip);
    return 0;
}