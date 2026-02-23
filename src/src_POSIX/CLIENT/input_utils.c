#include "input_utils.h"
#include <stdio.h>
#include <string.h>

/*
 *  Funzione blindata per la lettura da stdin.
 * Sostituisce l'uso pericoloso di fgets().
 * * - Mostra il prompt all'utente.
 * - Legge l'input in modo sicuro.
 * - Se l'utente digita troppi caratteri, svuota il buffer di sistema 
 * (evitando l'effetto valanga sui menu successivi).
 * - Sostituisce i '|' con spazi per proteggere il protocollo.
 * * Ritorna 1 in caso di successo, 0 in caso di EOF/Errore.
 */
int read_input(const char *prompt, char *buffer, size_t size) {
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout); /* Forza la stampa del prompt a schermo */
    }

    if (!fgets(buffer, size, stdin)) {
        return 0; /* EOF o errore (es. Ctrl+D) */
    }

    /* Controlla se la stringa contiene il newline */
    char *newline = strchr(buffer, '\n');
    if (newline) {
        *newline = '\0'; /* Rimuove il newline */
    } else {
        /* Il buffer è pieno, ma c'è ancora qualcosa nello stdin. 
           Dobbiamo svuotare lo stream per non corrompere la prossima lettura */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    /* Sanificazione input: impedisce l'uso del separatore di protocollo '|' */
    for (size_t i = 0; buffer[i]; i++) {
        if (buffer[i] == '|')
            buffer[i] = ' ';
    }

    return 1;
}