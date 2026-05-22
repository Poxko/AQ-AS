#ifndef FILTRI_H
#define FILTRI_H

#include "richiesta.h"

/* Copia in destinazione le richieste con lo stato richiesto; restituisce il conteggio */
int filtraPerStato(const Richiesta *richieste, int numeroRichieste,
                   Richiesta *destinazione, Stato stato);

/* Copia in destinazione le richieste con la priorità richiesta; restituisce il conteggio */
int filtraPerPriorita(const Richiesta *richieste, int numeroRichieste,
                      Richiesta *destinazione, Priorita priorita);

/* Copia in destinazione le richieste il cui cliente contiene la sottostringa */
int filtraPerCliente(const Richiesta *richieste, int numeroRichieste,
                     Richiesta *destinazione, const char *nomeCliente);

/* Calcola contatore, costo medio stimato e costo massimo per una tipologia di dispositivo */
void statistichePerDispositivo(const Richiesta *richieste, int numeroRichieste,
                               const char *dispositivo, int *contatore,
                               float *costoMedio, float *costoMassimo);

/* Compila l'array conteggi con il numero di richieste per ogni livello di priorità */
void statistichePerPriorita(const Richiesta *richieste, int numeroRichieste,
                            int conteggi[NUM_PRIORITA]);

#endif /* FILTRI_H */
