#include "input_utils.h"
#include <string.h>


/**
 * @brief Pulisce l'input dell'utente.
 *
 * La funzione rimuove il carattere di newline finale e sostituisce
 * eventuali caratteri '|' con spazi per evitare la rottura
 * del protocollo testuale client-server.
 *
 * @param str Stringa da pulire.
 */
void clean_input(char *str) {
    if (!str)
        return;

    str[strcspn(str, "\n")] = '\0';

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|')
            str[i] = ' ';
    }
}
