#ifndef ORDINAMENTO_H
#define ORDINAMENTO_H

#include "richiesta.h"

/* Tipo di funzione comparatore tra due richieste (stile qsort) */
typedef int (*ComparatoreRichieste)(const Richiesta *a, const Richiesta *b);

/* Copia l'array sorgente in destinazione senza modificarlo */
void copiaArrayRichieste(const Richiesta *sorgente, Richiesta *destinazione,
                         int numeroRichieste);

/* Ordina una copia per costo stimato crescente usando Quick Sort */
void ordinaPerCostoStimato(Richiesta *copia, int numeroRichieste);

/* Ordina una copia per data di apertura crescente usando Merge Sort */
void ordinaPerDataApertura(Richiesta *copia, int numeroRichieste);

#endif /* ORDINAMENTO_H */
