#include "gestione.h"

#include <stdio.h>
#include <string.h>

/* Verifica che una stringa non sia vuota */
static int stringaValida(const char *stringa)
{
    return stringa != NULL && stringa[0] != '\0';
}

/* Verifica che un costo stimato sia non negativo */
static int costoStimatoValido(float costo)
{
    return costo >= 0.0f;
}

/* Verifica che un costo finale sia non negativo */
static int costoFinaleValido(float costo)
{
    return costo >= 0.0f;
}

/* Calcola il prossimo codice disponibile (massimo esistente + 1) */
static int generaNuovoCodice(const Richiesta *richieste, int numeroRichieste)
{
    int i;
    int massimo = 0;

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].codice > massimo) {
            massimo = richieste[i].codice;
        }
    }

    return massimo + 1;
}

/* Restituisce il nome testuale di una priorità */
static const char *nomePriorita(Priorita priorita)
{
    switch (priorita) {
        case BASSA:  return "BASSA";
        case MEDIA:  return "MEDIA";
        case ALTA:   return "ALTA";
        default:     return "?";
    }
}

/* Restituisce il nome testuale di uno stato */
static const char *nomeStato(Stato stato)
{
    switch (stato) {
        case APERTA:           return "APERTA";
        case IN_LAVORAZIONE:   return "IN_LAVORAZIONE";
        case COMPLETATA:       return "COMPLETATA";
        case ANNULLATA:        return "ANNULLATA";
        default:               return "?";
    }
}

/* Inserisce una nuova richiesta con codice generato automaticamente */
int inserisciRichiesta(Richiesta *richieste, int *numeroRichieste,
                       const char *cliente, const char *dispositivo,
                       const char *descrizione, Priorita priorita,
                       float costoStimato, const char *dataApertura)
{
    Richiesta nuova;

    if (richieste == NULL || numeroRichieste == NULL) {
        return -1;
    }

    if (*numeroRichieste >= MAX_RICHIESTE) {
        return -1;
    }

    if (!stringaValida(cliente) || !stringaValida(dispositivo) ||
        !stringaValida(descrizione) || !stringaValida(dataApertura)) {
        return -1;
    }

    if (!costoStimatoValido(costoStimato)) {
        return -1;
    }

    nuova.codice = generaNuovoCodice(richieste, *numeroRichieste);
    strncpy(nuova.cliente, cliente, MAX_CLIENTE - 1);
    nuova.cliente[MAX_CLIENTE - 1] = '\0';
    strncpy(nuova.dispositivo, dispositivo, MAX_DISPOSITIVO - 1);
    nuova.dispositivo[MAX_DISPOSITIVO - 1] = '\0';
    strncpy(nuova.descrizione, descrizione, MAX_DESCRIZIONE - 1);
    nuova.descrizione[MAX_DESCRIZIONE - 1] = '\0';
    nuova.priorita = priorita;
    nuova.stato = APERTA;
    nuova.costoStimato = costoStimato;
    nuova.costoFinale = COSTO_FINALE_NON_DISPONIBILE;
    strncpy(nuova.dataApertura, dataApertura, MAX_DATA - 1);
    nuova.dataApertura[MAX_DATA - 1] = '\0';

    richieste[*numeroRichieste] = nuova;
    (*numeroRichieste)++;

    return 0;
}

/* Stampa a video tutte le richieste presenti nell'array */
void visualizzaRichieste(const Richiesta *richieste, int numeroRichieste)
{
    int i;

    if (richieste == NULL || numeroRichieste <= 0) {
        printf("\nNessuna richiesta presente.\n");
        return;
    }

    printf("\n--- ELENCO RICHIESTE (%d) ---\n", numeroRichieste);
    for (i = 0; i < numeroRichieste; i++) {
        visualizzaRichiesta(&richieste[i]);
        printf("----------------------------------------\n");
    }
}

/* Stampa a video il dettaglio di una singola richiesta */
void visualizzaRichiesta(const Richiesta *richiesta)
{
    if (richiesta == NULL) {
        return;
    }

    printf("Codice: %d\n", richiesta->codice);
    printf("Cliente: %s\n", richiesta->cliente);
    printf("Dispositivo: %s\n", richiesta->dispositivo);
    printf("Descrizione: %s\n", richiesta->descrizione);
    printf("Priorita: %s\n", nomePriorita(richiesta->priorita));
    printf("Stato: %s\n", nomeStato(richiesta->stato));
    printf("Costo stimato: %.2f EUR\n", richiesta->costoStimato);

    if (richiesta->costoFinale < 0.0f) {
        printf("Costo finale: non disponibile\n");
    } else {
        printf("Costo finale: %.2f EUR\n", richiesta->costoFinale);
    }

    printf("Data apertura: %s\n", richiesta->dataApertura);
}

/* Restituisce l'indice della richiesta con il codice indicato, oppure -1 */
int cercaRichiestaPerCodice(const Richiesta *richieste, int numeroRichieste, int codice)
{
    int i;

    if (richieste == NULL || codice <= 0) {
        return -1;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].codice == codice) {
            return i;
        }
    }

    return -1;
}

/* Modifica stato, descrizione e costo stimato di una richiesta esistente */
int modificaRichiesta(Richiesta *richieste, int numeroRichieste, int codice,
                      Stato nuovoStato, const char *nuovaDescrizione,
                      float nuovoCostoStimato)
{
    int indice;

    if (richieste == NULL) {
        return -1;
    }

    indice = cercaRichiestaPerCodice(richieste, numeroRichieste, codice);
    if (indice == -1) {
        return -1;
    }

    if (!stringaValida(nuovaDescrizione) || !costoStimatoValido(nuovoCostoStimato)) {
        return -1;
    }

    if (richieste[indice].stato == ANNULLATA) {
        return -1;
    }

    richieste[indice].stato = nuovoStato;
    strncpy(richieste[indice].descrizione, nuovaDescrizione, MAX_DESCRIZIONE - 1);
    richieste[indice].descrizione[MAX_DESCRIZIONE - 1] = '\0';
    richieste[indice].costoStimato = nuovoCostoStimato;

    return 0;
}

/* Imposta il costo finale di una richiesta completata */
int aggiornaCostoFinale(Richiesta *richieste, int numeroRichieste,
                        int codice, float costoFinale)
{
    int indice;

    if (richieste == NULL || !costoFinaleValido(costoFinale)) {
        return -1;
    }

    indice = cercaRichiestaPerCodice(richieste, numeroRichieste, codice);
    if (indice == -1 || richieste[indice].stato != COMPLETATA) {
        return -1;
    }

    richieste[indice].costoFinale = costoFinale;
    return 0;
}

/* Esegue l'annullamento soft impostando lo stato ANNULLATA */
int annullaRichiesta(Richiesta *richieste, int numeroRichieste, int codice)
{
    int indice;

    if (richieste == NULL) {
        return -1;
    }

    indice = cercaRichiestaPerCodice(richieste, numeroRichieste, codice);
    if (indice == -1 || richieste[indice].stato == ANNULLATA) {
        return -1;
    }

    richieste[indice].stato = ANNULLATA;
    return 0;
}
