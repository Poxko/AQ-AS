#include "filtri.h"

#include <string.h>
#include <ctype.h>

/* Confronta due stringhe ignorando maiuscole/minuscole */
static int stringheUgualiIgnoreCase(const char *a, const char *b)
{
    size_t i;
    size_t len;

    if (a == NULL || b == NULL) {
        return 0;
    }

    len = strlen(a);
    if (len != strlen(b)) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) {
            return 0;
        }
    }

    return 1;
}

/* Verifica se testo contiene sottostringa (case-insensitive) */
static int contieneSottostringa(const char *testo, const char *sottostringa)
{
    size_t lenTesto;
    size_t lenSotto;
    size_t i;
    size_t j;

    if (testo == NULL || sottostringa == NULL || sottostringa[0] == '\0') {
        return 0;
    }

    lenTesto = strlen(testo);
    lenSotto = strlen(sottostringa);
    if (lenSotto > lenTesto) {
        return 0;
    }

    for (i = 0; i <= lenTesto - lenSotto; i++) {
        int corrispondenza = 1;
        for (j = 0; j < lenSotto; j++) {
            if (tolower((unsigned char)testo[i + j]) !=
                tolower((unsigned char)sottostringa[j])) {
                corrispondenza = 0;
                break;
            }
        }
        if (corrispondenza) {
            return 1;
        }
    }

    return 0;
}

/* Copia in destinazione le richieste con lo stato richiesto; restituisce il conteggio */
int filtraPerStato(const Richiesta *richieste, int numeroRichieste,
                   Richiesta *destinazione, Stato stato)
{
    int i;
    int contatore = 0;

    if (richieste == NULL || destinazione == NULL || numeroRichieste < 0) {
        return 0;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].stato == stato) {
            destinazione[contatore++] = richieste[i];
        }
    }

    return contatore;
}

/* Copia in destinazione le richieste con la priorità richiesta; restituisce il conteggio */
int filtraPerPriorita(const Richiesta *richieste, int numeroRichieste,
                      Richiesta *destinazione, Priorita priorita)
{
    int i;
    int contatore = 0;

    if (richieste == NULL || destinazione == NULL || numeroRichieste < 0) {
        return 0;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].priorita == priorita) {
            destinazione[contatore++] = richieste[i];
        }
    }

    return contatore;
}

/* Copia in destinazione le richieste il cui cliente contiene la sottostringa */
int filtraPerCliente(const Richiesta *richieste, int numeroRichieste,
                     Richiesta *destinazione, const char *nomeCliente)
{
    int i;
    int contatore = 0;

    if (richieste == NULL || destinazione == NULL || nomeCliente == NULL ||
        nomeCliente[0] == '\0' || numeroRichieste < 0) {
        return 0;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (contieneSottostringa(richieste[i].cliente, nomeCliente)) {
            destinazione[contatore++] = richieste[i];
        }
    }

    return contatore;
}

/* Calcola contatore, costo medio stimato e costo massimo per una tipologia di dispositivo */
void statistichePerDispositivo(const Richiesta *richieste, int numeroRichieste,
                               const char *dispositivo, int *contatore,
                               float *costoMedio, float *costoMassimo)
{
    int i;
    int trovate = 0;
    float somma = 0.0f;
    float massimo = 0.0f;

    if (contatore != NULL) {
        *contatore = 0;
    }
    if (costoMedio != NULL) {
        *costoMedio = 0.0f;
    }
    if (costoMassimo != NULL) {
        *costoMassimo = 0.0f;
    }

    if (richieste == NULL || dispositivo == NULL || dispositivo[0] == '\0') {
        return;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (stringheUgualiIgnoreCase(richieste[i].dispositivo, dispositivo)) {
            trovate++;
            somma += richieste[i].costoStimato;
            if (trovate == 1 || richieste[i].costoStimato > massimo) {
                massimo = richieste[i].costoStimato;
            }
        }
    }

    if (contatore != NULL) {
        *contatore = trovate;
    }

    if (trovate > 0) {
        if (costoMedio != NULL) {
            *costoMedio = somma / (float)trovate;
        }
        if (costoMassimo != NULL) {
            *costoMassimo = massimo;
        }
    }
}

/* Compila l'array conteggi con il numero di richieste per ogni livello di priorità */
void statistichePerPriorita(const Richiesta *richieste, int numeroRichieste,
                            int conteggi[NUM_PRIORITA])
{
    int i;

    if (conteggi == NULL) {
        return;
    }

    conteggi[BASSA] = 0;
    conteggi[MEDIA] = 0;
    conteggi[ALTA] = 0;

    if (richieste == NULL || numeroRichieste <= 0) {
        return;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].priorita >= BASSA && richieste[i].priorita <= ALTA) {
            conteggi[richieste[i].priorita]++;
        }
    }
}
