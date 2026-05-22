#ifndef FILEIO_H
#define FILEIO_H

#include "richiesta.h"

/**
 * Carica tutte le richieste da FILE_RICHIESTE; se il file non esiste l'array resta vuoto.
 * @param richieste array destinazione
 * @param numeroRichieste puntatore al contatore (impostato al numero di record caricati)
 * @return 0 in caso di successo, -1 in caso di errore di lettura
 */
int caricaRichieste(Richiesta *richieste, int *numeroRichieste);

/**
 * Salva tutte le richieste su FILE_RICHIESTE sovrascrivendo il file esistente.
 * @param richieste array sorgente
 * @param numeroRichieste numero di record da salvare
 * @return 0 in caso di successo, -1 in caso di errore di scrittura
 */
int salvaRichieste(const Richiesta *richieste, int numeroRichieste);

#endif /* FILEIO_H */
