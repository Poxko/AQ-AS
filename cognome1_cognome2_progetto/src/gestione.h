#ifndef GESTIONE_H
#define GESTIONE_H

#include "richiesta.h"

/* Inserisce una nuova richiesta con codice generato automaticamente */
int inserisciRichiesta(Richiesta *richieste, int *numeroRichieste,
                       const char *cliente, const char *dispositivo,
                       const char *descrizione, Priorita priorita,
                       float costoStimato, const char *dataApertura);

/* Stampa a video tutte le richieste presenti nell'array */
void visualizzaRichieste(const Richiesta *richieste, int numeroRichieste);

/* Stampa a video il dettaglio di una singola richiesta */
void visualizzaRichiesta(const Richiesta *richiesta);

/* Restituisce l'indice della richiesta con il codice indicato, oppure -1 */
int cercaRichiestaPerCodice(const Richiesta *richieste, int numeroRichieste, int codice);

/* Modifica stato, descrizione e costo stimato di una richiesta esistente */
int modificaRichiesta(Richiesta *richieste, int numeroRichieste, int codice,
                      Stato nuovoStato, const char *nuovaDescrizione,
                      float nuovoCostoStimato);

/* Imposta il costo finale di una richiesta completata */
int aggiornaCostoFinale(Richiesta *richieste, int numeroRichieste,
                        int codice, float costoFinale);

/* Esegue l'annullamento soft impostando lo stato ANNULLATA */
int annullaRichiesta(Richiesta *richieste, int numeroRichieste, int codice);

#endif /* GESTIONE_H */
