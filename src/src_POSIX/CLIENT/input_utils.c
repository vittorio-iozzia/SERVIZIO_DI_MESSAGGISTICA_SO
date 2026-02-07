#include "input_utils.h"
#include <string.h>

/*
 * Pulisce una stringa letta da input.
 *
 * - Rimuove il carattere di newline finale
 * - Sostituisce il separatore di protocollo '|' con uno spazio
 *   per evitare input malformati lato client
 */
void clean_input(char *str) {

    if (!str) return;

    /* Rimozione newline finale */
    str[strcspn(str, "\n")] = 0;

    /* Sanificazione input: impedisce l'uso del separatore di protocollo */
    for (int i = 0; str[i]; i++) {
        if (str[i] == '|')
            str[i] = ' ';
    }
}
