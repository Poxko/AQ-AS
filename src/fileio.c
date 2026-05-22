#include "fileio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LUNGHEZZA_RIGA 512
#define EPSILON_COSTO 0.005f

/* Converte una stringa in valore Priorita; restituisce 1 se riconosciuta */
static int stringaToPriorita(const char *stringa, Priorita *priorita)
{
    if (strcmp(stringa, "BASSA") == 0) {
        *priorita = BASSA;
        return 1;
    }
    if (strcmp(stringa, "MEDIA") == 0) {
        *priorita = MEDIA;
        return 1;
    }
    if (strcmp(stringa, "ALTA") == 0) {
        *priorita = ALTA;
        return 1;
    }
    return 0;
}

/* Converte una stringa in valore Stato; restituisce 1 se riconosciuto */
static int stringaToStato(const char *stringa, Stato *stato)
{
    if (strcmp(stringa, "APERTA") == 0) {
        *stato = APERTA;
        return 1;
    }
    if (strcmp(stringa, "IN_LAVORAZIONE") == 0) {
        *stato = IN_LAVORAZIONE;
        return 1;
    }
    if (strcmp(stringa, "COMPLETATA") == 0) {
        *stato = COMPLETATA;
        return 1;
    }
    if (strcmp(stringa, "ANNULLATA") == 0) {
        *stato = ANNULLATA;
        return 1;
    }
    return 0;
}

/* Restituisce la stringa corrispondente a una Priorita */
static const char *prioritaToString(Priorita priorita)
{
    switch (priorita) {
        case BASSA:  return "BASSA";
        case MEDIA:  return "MEDIA";
        case ALTA:   return "ALTA";
        default:     return "BASSA";
    }
}

/* Restituisce la stringa corrispondente a uno Stato */
static const char *statoToString(Stato stato)
{
    switch (stato) {
        case APERTA:           return "APERTA";
        case IN_LAVORAZIONE:   return "IN_LAVORAZIONE";
        case COMPLETATA:       return "COMPLETATA";
        case ANNULLATA:        return "ANNULLATA";
        default:               return "APERTA";
    }
}

/* Verifica che una stringa non sia vuota (ignorando spazi iniziali e finali) */
static int stringaNonVuota(const char *stringa)
{
    size_t inizio;
    size_t fine;

    if (stringa == NULL) {
        return 0;
    }

    inizio = 0;
    while (stringa[inizio] == ' ' || stringa[inizio] == '\t') {
        inizio++;
    }

    fine = strlen(stringa);
    while (fine > inizio && (stringa[fine - 1] == ' ' || stringa[fine - 1] == '\t')) {
        fine--;
    }

    return fine > inizio;
}

/* Verifica il formato della data GG/MM/AAAA */
static int dataValida(const char *data)
{
    int giorno;
    int mese;
    int anno;
    size_t i;

    if (data == NULL || strlen(data) != 10) {
        return 0;
    }

    if (data[2] != '/' || data[5] != '/') {
        return 0;
    }

    if (sscanf(data, "%d/%d/%d", &giorno, &mese, &anno) != 3) {
        return 0;
    }

    if (giorno < 1 || giorno > 31 || mese < 1 || mese > 12 || anno < 1900) {
        return 0;
    }

    for (i = 0; data[i] != '\0'; i++) {
        if (i == 2 || i == 5) {
            continue;
        }
        if (data[i] < '0' || data[i] > '9') {
            return 0;
        }
    }

    return 1;
}

/* Verifica che un costo sia non negativo oppure uguale al sentinel del costo finale */
static int costoValido(float costo, int ammetteSentinel)
{
    if (ammetteSentinel && fabsf(costo - COSTO_FINALE_NON_DISPONIBILE) < EPSILON_COSTO) {
        return 1;
    }
    return costo >= 0.0f;
}

/* Verifica se un codice è già presente nell'array caricato */
static int codiceDuplicato(const Richiesta *richieste, int numeroRichieste, int codice)
{
    int i;

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].codice == codice) {
            return 1;
        }
    }
    return 0;
}

/* Analizza una riga del file e compila una Richiesta; restituisce 1 se valida */
static int parseRiga(const char *linea, Richiesta *richiesta)
{
    char buffer[LUNGHEZZA_RIGA];
    char *token;

    if (linea == NULL || richiesta == NULL || strlen(linea) >= LUNGHEZZA_RIGA) {
        return 0;
    }

    strncpy(buffer, linea, LUNGHEZZA_RIGA - 1);
    buffer[LUNGHEZZA_RIGA - 1] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (!stringaNonVuota(buffer)) {
        return 0;
    }

    token = strtok(buffer, "|");
    if (token == NULL) {
        return 0;
    }
    richiesta->codice = atoi(token);
    if (richiesta->codice <= 0) {
        return 0;
    }

    token = strtok(NULL, "|");
    if (token == NULL || !stringaNonVuota(token)) {
        return 0;
    }
    strncpy(richiesta->cliente, token, MAX_CLIENTE - 1);
    richiesta->cliente[MAX_CLIENTE - 1] = '\0';

    token = strtok(NULL, "|");
    if (token == NULL || !stringaNonVuota(token)) {
        return 0;
    }
    strncpy(richiesta->dispositivo, token, MAX_DISPOSITIVO - 1);
    richiesta->dispositivo[MAX_DISPOSITIVO - 1] = '\0';

    token = strtok(NULL, "|");
    if (token == NULL || !stringaNonVuota(token)) {
        return 0;
    }
    strncpy(richiesta->descrizione, token, MAX_DESCRIZIONE - 1);
    richiesta->descrizione[MAX_DESCRIZIONE - 1] = '\0';

    token = strtok(NULL, "|");
    if (token == NULL || !stringaToPriorita(token, &richiesta->priorita)) {
        return 0;
    }

    token = strtok(NULL, "|");
    if (token == NULL || !stringaToStato(token, &richiesta->stato)) {
        return 0;
    }

    token = strtok(NULL, "|");
    if (token == NULL) {
        return 0;
    }
    richiesta->costoStimato = (float)atof(token);
    if (!costoValido(richiesta->costoStimato, 0)) {
        return 0;
    }

    token = strtok(NULL, "|");
    if (token == NULL) {
        return 0;
    }
    richiesta->costoFinale = (float)atof(token);
    if (!costoValido(richiesta->costoFinale, 1)) {
        return 0;
    }

    token = strtok(NULL, "|");
    if (token == NULL || !dataValida(token)) {
        return 0;
    }
    strncpy(richiesta->dataApertura, token, MAX_DATA - 1);
    richiesta->dataApertura[MAX_DATA - 1] = '\0';

    return strtok(NULL, "|") == NULL;
}

/* Carica tutte le richieste da FILE_RICHIESTE; se il file non esiste l'array resta vuoto */
int caricaRichieste(Richiesta *richieste, int *numeroRichieste)
{
    FILE *file;
    char linea[LUNGHEZZA_RIGA];
    Richiesta temporanea;

    if (richieste == NULL || numeroRichieste == NULL) {
        return -1;
    }

    *numeroRichieste = 0;

    file = fopen(FILE_RICHIESTE, "r");
    if (file == NULL) {
        return 0;
    }

    while (fgets(linea, sizeof(linea), file) != NULL) {
        if (*numeroRichieste >= MAX_RICHIESTE) {
            fclose(file);
            return -1;
        }

        if (!parseRiga(linea, &temporanea)) {
            continue;
        }

        if (codiceDuplicato(richieste, *numeroRichieste, temporanea.codice)) {
            continue;
        }

        richieste[*numeroRichieste] = temporanea;
        (*numeroRichieste)++;
    }

    if (ferror(file)) {
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

/* Salva tutte le richieste su FILE_RICHIESTE sovrascrivendo il file esistente */
int salvaRichieste(const Richiesta *richieste, int numeroRichieste)
{
    FILE *file;
    int i;

    if (richieste == NULL || numeroRichieste < 0 || numeroRichieste > MAX_RICHIESTE) {
        return -1;
    }

    file = fopen(FILE_RICHIESTE, "w");
    if (file == NULL) {
        return -1;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (fprintf(file, "%d|%s|%s|%s|%s|%s|%.2f|%.2f|%s\n",
                    richieste[i].codice,
                    richieste[i].cliente,
                    richieste[i].dispositivo,
                    richieste[i].descrizione,
                    prioritaToString(richieste[i].priorita),
                    statoToString(richieste[i].stato),
                    richieste[i].costoStimato,
                    richieste[i].costoFinale,
                    richieste[i].dataApertura) < 0) {
            fclose(file);
            return -1;
        }
    }

    if (fclose(file) != 0) {
        return -1;
    }

    return 0;
}
