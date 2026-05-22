#ifndef FILEIO_H
#define FILEIO_H

#include "richiesta.h"

/* Carica tutte le richieste da FILE_RICHIESTE; se il file non esiste l'array resta vuoto */
int caricaRichieste(Richiesta *richieste, int *numeroRichieste);

/* Salva tutte le richieste su FILE_RICHIESTE sovrascrivendo il file esistente */
int salvaRichieste(const Richiesta *richieste, int numeroRichieste);

#endif /* FILEIO_H */
