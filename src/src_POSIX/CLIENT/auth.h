#ifndef AUTH_H
#define AUTH_H

/*
 * Avvia la procedura di autenticazione lato client.
 *
 * La funzione gestisce l'interazione con l'utente (login/registrazione)
 * e comunica con il server tramite socket usando il protocollo testuale.
 *
 * Parametri:
 *  - sock: socket TCP già connessa al server
 *
 * Valore di ritorno:
 *  - 0  se l'autenticazione va a buon fine
 *  - -1 in caso di uscita volontaria o errore
 */
int client_authenticate(int sock);

#endif /* AUTH_H */
