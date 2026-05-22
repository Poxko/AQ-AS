#include "report.h"

#include "ordinamento.h"
#include "filtri.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define EPSILON_COSTO 0.005f
#define MAX_TIPI_DISPOSITIVO 50

/* Restituisce il nome testuale di uno stato */
static const char *nomeStato(Stato stato)
{
    switch (stato) {
        case APERTA:         return "APERTA";
        case IN_LAVORAZIONE: return "IN_LAVORAZIONE";
        case COMPLETATA:     return "COMPLETATA";
        case ANNULLATA:      return "ANNULLATA";
        default:             return "?";
    }
}

/* Restituisce il nome testuale di una priorità */
static const char *nomePriorita(Priorita priorita)
{
    switch (priorita) {
        case BASSA: return "BASSA";
        case MEDIA: return "MEDIA";
        case ALTA:  return "ALTA";
        default:    return "?";
    }
}

/* Verifica se il costo finale è valorizzato (diverso dal sentinel) */
static int costoFinaleDisponibile(float costoFinale)
{
    return fabsf(costoFinale - COSTO_FINALE_NON_DISPONIBILE) >= EPSILON_COSTO;
}

/* Scrive una riga di riepilogo di una richiesta su file */
static void scriviRichiestaSuFile(FILE *file, const Richiesta *richiesta)
{
    fprintf(file, "  [%d] %s | %s | %s | %s | %s | stimato: %.2f",
            richiesta->codice,
            richiesta->cliente,
            richiesta->dispositivo,
            richiesta->descrizione,
            nomePriorita(richiesta->priorita),
            nomeStato(richiesta->stato),
            richiesta->costoStimato);

    if (costoFinaleDisponibile(richiesta->costoFinale)) {
        fprintf(file, " | finale: %.2f", richiesta->costoFinale);
    } else {
        fprintf(file, " | finale: N/D");
    }

    fprintf(file, " | %s\n", richiesta->dataApertura);
}

/* Calcola il costo medio stimato su tutte le richieste */
static float calcolaCostoMedioStimato(const Richiesta *richieste, int numeroRichieste)
{
    int i;
    float somma = 0.0f;

    if (numeroRichieste <= 0) {
        return 0.0f;
    }

    for (i = 0; i < numeroRichieste; i++) {
        somma += richieste[i].costoStimato;
    }

    return somma / (float)numeroRichieste;
}

/* Calcola il costo medio finale sulle sole richieste completate con costo disponibile */
static float calcolaCostoMedioFinale(const Richiesta *richieste, int numeroRichieste)
{
    int i;
    int contatore = 0;
    float somma = 0.0f;

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].stato == COMPLETATA &&
            costoFinaleDisponibile(richieste[i].costoFinale)) {
            somma += richieste[i].costoFinale;
            contatore++;
        }
    }

    return contatore == 0 ? 0.0f : somma / (float)contatore;
}

/* Conta le richieste per ogni stato */
static void contaPerStato(const Richiesta *richieste, int numeroRichieste, int conteggi[NUM_STATI])
{
    int i;

    for (i = 0; i < NUM_STATI; i++) {
        conteggi[i] = 0;
    }

    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].stato >= APERTA && richieste[i].stato <= ANNULLATA) {
            conteggi[richieste[i].stato]++;
        }
    }
}

/* Aggiunge un tipo dispositivo all'elenco se non già presente */
static void aggiungiTipoDispositivo(char tipi[][MAX_DISPOSITIVO], int *numeroTipi,
                                    const char *dispositivo)
{
    int i;

    for (i = 0; i < *numeroTipi; i++) {
        if (strcmp(tipi[i], dispositivo) == 0) {
            return;
        }
    }

    if (*numeroTipi < MAX_TIPI_DISPOSITIVO) {
        strncpy(tipi[*numeroTipi], dispositivo, MAX_DISPOSITIVO - 1);
        tipi[*numeroTipi][MAX_DISPOSITIVO - 1] = '\0';
        (*numeroTipi)++;
    }
}

/* Genera il report generale su FILE_REPORT1 */
int generaReportGenerale(const Richiesta *richieste, int numeroRichieste)
{
    FILE *file;
    Richiesta copia[MAX_RICHIESTE];
    int conteggiStato[NUM_STATI];
    int conteggiPriorita[NUM_PRIORITA];
    int i;

    file = fopen(FILE_REPORT1, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "========================================\n");
    fprintf(file, "       REPORT 1 - GENERALE\n");
    fprintf(file, "========================================\n\n");
    fprintf(file, "Totale richieste: %d\n\n", numeroRichieste);

    contaPerStato(richieste, numeroRichieste, conteggiStato);
    fprintf(file, "Richieste per stato:\n");
    for (i = 0; i < NUM_STATI; i++) {
        fprintf(file, "  - %s: %d\n", nomeStato((Stato)i), conteggiStato[i]);
    }
    fprintf(file, "\n");

    statistichePerPriorita(richieste, numeroRichieste, conteggiPriorita);
    fprintf(file, "Richieste per priorita:\n");
    for (i = 0; i < NUM_PRIORITA; i++) {
        fprintf(file, "  - %s: %d\n", nomePriorita((Priorita)i), conteggiPriorita[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "Costo medio stimato (tutte): %.2f EUR\n",
            calcolaCostoMedioStimato(richieste, numeroRichieste));
    fprintf(file, "Costo medio finale (completate): %.2f EUR\n\n",
            calcolaCostoMedioFinale(richieste, numeroRichieste));

    if (numeroRichieste > 0) {
        copiaArrayRichieste(richieste, copia, numeroRichieste);
        ordinaPerCostoStimato(copia, numeroRichieste);
        fprintf(file, "Elenco ordinato per costo stimato (crescente):\n");
        for (i = 0; i < numeroRichieste; i++) {
            scriviRichiestaSuFile(file, &copia[i]);
        }
    } else {
        fprintf(file, "Nessuna richiesta da elencare.\n");
    }

    fclose(file);
    return 0;
}

/* Genera il report operativo su FILE_REPORT2 */
int generaReportOperativo(const Richiesta *richieste, int numeroRichieste)
{
    FILE *file;
    Richiesta copia[MAX_RICHIESTE];
    char tipiDispositivo[MAX_TIPI_DISPOSITIVO][MAX_DISPOSITIVO];
    int numeroTipi = 0;
    int filtrate;
    int i;
    int t;
    int contatore;
    float medio;
    float massimo;

    file = fopen(FILE_REPORT2, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(file, "========================================\n");
    fprintf(file, "       REPORT 2 - OPERATIVO\n");
    fprintf(file, "========================================\n\n");

    fprintf(file, ">>> Richieste ALTA priorita ancora APERTE:\n");
    filtrate = 0;
    for (i = 0; i < numeroRichieste; i++) {
        if (richieste[i].priorita == ALTA && richieste[i].stato == APERTA) {
            scriviRichiestaSuFile(file, &richieste[i]);
            filtrate++;
        }
    }
    if (filtrate == 0) {
        fprintf(file, "  (nessuna)\n");
    }
    fprintf(file, "\n");

    fprintf(file, ">>> Richieste IN LAVORAZIONE:\n");
    filtrate = filtraPerStato(richieste, numeroRichieste, copia, IN_LAVORAZIONE);
    if (filtrate == 0) {
        fprintf(file, "  (nessuna)\n");
    } else {
        for (i = 0; i < filtrate; i++) {
            scriviRichiestaSuFile(file, &copia[i]);
        }
    }
    fprintf(file, "\n");

    fprintf(file, ">>> Richieste COMPLETATE:\n");
    filtrate = filtraPerStato(richieste, numeroRichieste, copia, COMPLETATA);
    if (filtrate == 0) {
        fprintf(file, "  (nessuna)\n");
    } else {
        for (i = 0; i < filtrate; i++) {
            scriviRichiestaSuFile(file, &copia[i]);
        }
    }
    fprintf(file, "\n");

    fprintf(file, ">>> Richieste ANNULLATE:\n");
    filtrate = filtraPerStato(richieste, numeroRichieste, copia, ANNULLATA);
    if (filtrate == 0) {
        fprintf(file, "  (nessuna)\n");
    } else {
        for (i = 0; i < filtrate; i++) {
            scriviRichiestaSuFile(file, &copia[i]);
        }
    }
    fprintf(file, "\n");

    fprintf(file, ">>> Riepilogo per tipologia dispositivo:\n");
    for (i = 0; i < numeroRichieste; i++) {
        aggiungiTipoDispositivo(tipiDispositivo, &numeroTipi, richieste[i].dispositivo);
    }

    if (numeroTipi == 0) {
        fprintf(file, "  (nessun dispositivo)\n");
    } else {
        for (t = 0; t < numeroTipi; t++) {
            statistichePerDispositivo(richieste, numeroRichieste, tipiDispositivo[t],
                                      &contatore, &medio, &massimo);
            fprintf(file, "  %s -> richieste: %d | costo medio stimato: %.2f EUR | "
                          "costo max stimato: %.2f EUR\n",
                    tipiDispositivo[t], contatore, medio, massimo);
        }
    }

    fclose(file);
    return 0;
}
