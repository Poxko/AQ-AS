#include "interfaccia.h"

#include "gestione.h"
#include "filtri.h"
#include "ordinamento.h"
#include "report.h"

#include <stdio.h>
#include <string.h>

#define LUNGHEZZA_INPUT 256

/* Svuota il buffer stdin dopo input non validi */
static void pulisciBufferInput(void)
{
    int carattere;

    while ((carattere = getchar()) != '\n' && carattere != EOF) {
    }
}

/* Legge una stringa non vuota da tastiera */
static int leggiStringa(const char *etichetta, char *destinazione, size_t dimensione)
{
    char buffer[LUNGHEZZA_INPUT];

    if (destinazione == NULL || dimensione == 0) {
        return 0;
    }

    printf("%s", etichetta);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (buffer[0] == '\0') {
        return 0;
    }

    strncpy(destinazione, buffer, dimensione - 1);
    destinazione[dimensione - 1] = '\0';
    return 1;
}

/* Legge un intero con messaggio e validazione minima */
static int leggiIntero(const char *etichetta, int minimo, int massimo, int *valore)
{
    int letto;
    int risultato;

    printf("%s", etichetta);
    risultato = scanf("%d", &letto);
    pulisciBufferInput();

    if (risultato != 1) {
        return 0;
    }

    if (letto < minimo || letto > massimo) {
        return 0;
    }

    *valore = letto;
    return 1;
}

/* Legge un float non negativo */
static int leggiFloatNonNegativo(const char *etichetta, float *valore)
{
    int risultato;
    float letto;

    printf("%s", etichetta);
    risultato = scanf("%f", &letto);
    pulisciBufferInput();

    if (risultato != 1 || letto < 0.0f) {
        return 0;
    }

    *valore = letto;
    return 1;
}

/* Legge una data nel formato GG/MM/AAAA */
static int leggiData(const char *etichetta, char *data)
{
    char buffer[LUNGHEZZA_INPUT];
    int giorno;
    int mese;
    int anno;

    printf("%s", etichetta);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strlen(buffer) != 10 || buffer[2] != '/' || buffer[5] != '/') {
        return 0;
    }

    if (sscanf(buffer, "%d/%d/%d", &giorno, &mese, &anno) != 3) {
        return 0;
    }

    if (giorno < 1 || giorno > 31 || mese < 1 || mese > 12 || anno < 1900) {
        return 0;
    }

    strncpy(data, buffer, MAX_DATA - 1);
    data[MAX_DATA - 1] = '\0';
    return 1;
}

/* Legge una priorità da tastiera (1=BASSA, 2=MEDIA, 3=ALTA) */
static int leggiPriorita(Priorita *priorita)
{
    int scelta;

    printf("Priorita (1=BASSA, 2=MEDIA, 3=ALTA): ");
    if (!leggiIntero("", 1, 3, &scelta)) {
        return 0;
    }

    switch (scelta) {
        case 1: *priorita = BASSA; return 1;
        case 2: *priorita = MEDIA; return 1;
        case 3: *priorita = ALTA;  return 1;
        default: return 0;
    }
}

/* Legge uno stato da tastiera */
static int leggiStato(Stato *stato)
{
    int scelta;

    printf("Stato (1=APERTA, 2=IN_LAVORAZIONE, 3=COMPLETATA, 4=ANNULLATA): ");
    if (!leggiIntero("", 1, 4, &scelta)) {
        return 0;
    }

    switch (scelta) {
        case 1: *stato = APERTA; return 1;
        case 2: *stato = IN_LAVORAZIONE; return 1;
        case 3: *stato = COMPLETATA; return 1;
        case 4: *stato = ANNULLATA; return 1;
        default: return 0;
    }
}

/* Mostra la schermata iniziale di benvenuto */
static void mostraSchermataIniziale(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("   CENTRO RIPARAZIONI - GESTIONE ASSISTENZA TECNICA\n");
    printf("============================================================\n");
    printf(" Progetto Laboratorio di Informatica\n");
    printf(" Dispositivi: smartphone, notebook, tablet, stampanti...\n");
    printf("============================================================\n\n");
}

/* Mostra il menu principale e restituisce la scelta */
static int menuPrincipale(void)
{
    int scelta = -1;

    printf("\n========== MENU PRINCIPALE ==========\n");
    printf("1. Gestione richieste\n");
    printf("2. Ricerca e filtri\n");
    printf("3. Ordinamenti\n");
    printf("4. Report\n");
    printf("5. Statistiche\n");
    printf("6. Salvataggio dati\n");
    printf("0. Esci dal programma\n");
    printf("===================================\n");

    if (!leggiIntero("Scelta: ", 0, 6, &scelta)) {
        printf("Errore: scelta non valida.\n");
        return -1;
    }

    return scelta;
}

/* Gestisce il sottomenu di inserimento richiesta */
static void gestisciInserimento(Richiesta *richieste, int *numeroRichieste)
{
    char cliente[MAX_CLIENTE];
    char dispositivo[MAX_DISPOSITIVO];
    char descrizione[MAX_DESCRIZIONE];
    char data[MAX_DATA];
    Priorita priorita;
    float costoStimato;

    if (!leggiStringa("Cliente: ", cliente, sizeof(cliente)) ||
        !leggiStringa("Dispositivo: ", dispositivo, sizeof(dispositivo)) ||
        !leggiStringa("Descrizione problema: ", descrizione, sizeof(descrizione)) ||
        !leggiPriorita(&priorita) ||
        !leggiFloatNonNegativo("Costo stimato (EUR): ", &costoStimato) ||
        !leggiData("Data apertura (GG/MM/AAAA): ", data)) {
        printf("Errore: dati non validi. Inserimento annullato.\n");
        return;
    }

    if (inserisciRichiesta(richieste, numeroRichieste, cliente, dispositivo,
                           descrizione, priorita, costoStimato, data) == 0) {
        printf("Conferma: richiesta inserita con successo.\n");
    } else {
        printf("Errore: impossibile inserire la richiesta.\n");
    }
}

/* Gestisce la modifica di una richiesta esistente */
static void gestisciModifica(Richiesta *richieste, int numeroRichieste)
{
    int codice;
    int indice;
    Stato stato;
    char descrizione[MAX_DESCRIZIONE];
    float costoStimato;

    if (!leggiIntero("Codice richiesta da modificare: ", 1, 999999, &codice)) {
        printf("Errore: codice non valido.\n");
        return;
    }

    indice = cercaRichiestaPerCodice(richieste, numeroRichieste, codice);
    if (indice == -1) {
        printf("Errore: richiesta non trovata.\n");
        return;
    }

    printf("\nDati attuali:\n");
    visualizzaRichiesta(&richieste[indice]);
    printf("\nInserisci i nuovi valori:\n");

    if (!leggiStato(&stato) ||
        !leggiStringa("Nuova descrizione: ", descrizione, sizeof(descrizione)) ||
        !leggiFloatNonNegativo("Nuovo costo stimato (EUR): ", &costoStimato)) {
        printf("Errore: dati non validi. Modifica annullata.\n");
        return;
    }

    if (modificaRichiesta(richieste, numeroRichieste, codice, stato,
                          descrizione, costoStimato) == 0) {
        printf("Conferma: richiesta modificata.\n");
    } else {
        printf("Errore: modifica non riuscita (richiesta annullata o dati errati).\n");
    }
}

/* Gestisce l'aggiornamento del costo finale */
static void gestisciCostoFinale(Richiesta *richieste, int numeroRichieste)
{
    int codice;
    float costoFinale;

    if (!leggiIntero("Codice richiesta completata: ", 1, 999999, &codice) ||
        !leggiFloatNonNegativo("Costo finale (EUR): ", &costoFinale)) {
        printf("Errore: dati non validi.\n");
        return;
    }

    if (aggiornaCostoFinale(richieste, numeroRichieste, codice, costoFinale) == 0) {
        printf("Conferma: costo finale aggiornato.\n");
    } else {
        printf("Errore: operazione consentita solo su richieste COMPLETATE.\n");
    }
}

/* Gestisce l'annullamento di una richiesta */
static void gestisciAnnullamento(Richiesta *richieste, int numeroRichieste)
{
    int codice;
    int conferma;

    if (!leggiIntero("Codice richiesta da annullare: ", 1, 999999, &codice)) {
        printf("Errore: codice non valido.\n");
        return;
    }

    printf("Confermi annullamento? (1=Si, 0=No): ");
    if (!leggiIntero("", 0, 1, &conferma) || conferma == 0) {
        printf("Operazione annullata dall'utente.\n");
        return;
    }

    if (annullaRichiesta(richieste, numeroRichieste, codice) == 0) {
        printf("Conferma: richiesta annullata (soft delete).\n");
    } else {
        printf("Errore: impossibile annullare la richiesta.\n");
    }
}

/* Sottomenu gestione richieste */
static void menuGestione(Richiesta *richieste, int *numeroRichieste)
{
    int scelta;

    do {
        printf("\n--- GESTIONE RICHIESTE ---\n");
        printf("1. Inserisci nuova richiesta\n");
        printf("2. Visualizza tutte le richieste\n");
        printf("3. Modifica richiesta\n");
        printf("4. Aggiorna costo finale\n");
        printf("5. Annulla richiesta\n");
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 5, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        switch (scelta) {
            case 1:
                gestisciInserimento(richieste, numeroRichieste);
                break;
            case 2:
                visualizzaRichieste(richieste, *numeroRichieste);
                break;
            case 3:
                gestisciModifica(richieste, *numeroRichieste);
                break;
            case 4:
                gestisciCostoFinale(richieste, *numeroRichieste);
                break;
            case 5:
                gestisciAnnullamento(richieste, *numeroRichieste);
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Stampa l'elenco di richieste filtrate */
static void mostraRisultatiFiltro(const Richiesta *risultati, int numero)
{
    int i;

    if (numero <= 0) {
        printf("Nessun risultato trovato.\n");
        return;
    }

    printf("\n--- RISULTATI FILTRO (%d) ---\n", numero);
    for (i = 0; i < numero; i++) {
        visualizzaRichiesta(&risultati[i]);
        printf("----------------------------------------\n");
    }
}

/* Sottomenu ricerca e filtri */
static void menuFiltri(const Richiesta *richieste, int numeroRichieste)
{
    Richiesta risultati[MAX_RICHIESTE];
    int scelta;
    int numeroFiltrate;
    Stato stato;
    Priorita priorita;
    char nomeCliente[MAX_CLIENTE];

    do {
        printf("\n--- RICERCA E FILTRI ---\n");
        printf("1. Cerca per codice\n");
        printf("2. Filtra per stato\n");
        printf("3. Filtra per priorita\n");
        printf("4. Filtra per nome cliente\n");
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 4, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        switch (scelta) {
            case 1: {
                int codice;
                int indice;
                if (leggiIntero("Codice: ", 1, 999999, &codice)) {
                    indice = cercaRichiestaPerCodice(richieste, numeroRichieste, codice);
                    if (indice == -1) {
                        printf("Errore: richiesta non trovata.\n");
                    } else {
                        visualizzaRichiesta(&richieste[indice]);
                    }
                } else {
                    printf("Errore: codice non valido.\n");
                }
                break;
            }
            case 2:
                if (leggiStato(&stato)) {
                    numeroFiltrate = filtraPerStato(richieste, numeroRichieste,
                                                    risultati, stato);
                    mostraRisultatiFiltro(risultati, numeroFiltrate);
                } else {
                    printf("Errore: stato non valido.\n");
                }
                break;
            case 3:
                if (leggiPriorita(&priorita)) {
                    numeroFiltrate = filtraPerPriorita(richieste, numeroRichieste,
                                                       risultati, priorita);
                    mostraRisultatiFiltro(risultati, numeroFiltrate);
                } else {
                    printf("Errore: priorita non valida.\n");
                }
                break;
            case 4:
                if (leggiStringa("Nome cliente (anche parziale): ",
                                 nomeCliente, sizeof(nomeCliente))) {
                    numeroFiltrate = filtraPerCliente(richieste, numeroRichieste,
                                                      risultati, nomeCliente);
                    mostraRisultatiFiltro(risultati, numeroFiltrate);
                } else {
                    printf("Errore: nome non valido.\n");
                }
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Sottomenu ordinamenti (sempre su copia) */
static void menuOrdinamenti(const Richiesta *richieste, int numeroRichieste)
{
    Richiesta copia[MAX_RICHIESTE];
    int scelta;

    do {
        printf("\n--- ORDINAMENTI (su copia) ---\n");
        printf("1. Ordina per costo stimato (Quick Sort)\n");
        printf("2. Ordina per data apertura (Merge Sort)\n");
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 2, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        if (numeroRichieste <= 0) {
            printf("Nessuna richiesta da ordinare.\n");
            continue;
        }

        switch (scelta) {
            case 1:
                copiaArrayRichieste(richieste, copia, numeroRichieste);
                ordinaPerCostoStimato(copia, numeroRichieste);
                printf("\nOrdinamento Quick Sort per costo stimato:\n");
                visualizzaRichieste(copia, numeroRichieste);
                break;
            case 2:
                copiaArrayRichieste(richieste, copia, numeroRichieste);
                ordinaPerDataApertura(copia, numeroRichieste);
                printf("\nOrdinamento Merge Sort per data apertura:\n");
                visualizzaRichieste(copia, numeroRichieste);
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Sottomenu report */
static void menuReport(const Richiesta *richieste, int numeroRichieste)
{
    int scelta;

    do {
        printf("\n--- REPORT ---\n");
        printf("1. Genera report generale (%s)\n", FILE_REPORT1);
        printf("2. Genera report operativo (%s)\n", FILE_REPORT2);
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 2, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        switch (scelta) {
            case 1:
                if (generaReportGenerale(richieste, numeroRichieste) == 0) {
                    printf("Conferma: report generale creato.\n");
                } else {
                    printf("Errore: impossibile creare il report.\n");
                }
                break;
            case 2:
                if (generaReportOperativo(richieste, numeroRichieste) == 0) {
                    printf("Conferma: report operativo creato.\n");
                } else {
                    printf("Errore: impossibile creare il report.\n");
                }
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Sottomenu statistiche */
static void menuStatistiche(const Richiesta *richieste, int numeroRichieste)
{
    int scelta;
    char dispositivo[MAX_DISPOSITIVO];
    int conteggi[NUM_PRIORITA];
    int contatore;
    float medio;
    float massimo;

    do {
        printf("\n--- STATISTICHE ---\n");
        printf("1. Statistiche per tipologia dispositivo\n");
        printf("2. Statistiche per priorita\n");
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 2, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        switch (scelta) {
            case 1:
                if (leggiStringa("Tipologia dispositivo: ", dispositivo,
                                 sizeof(dispositivo))) {
                    statistichePerDispositivo(richieste, numeroRichieste, dispositivo,
                                              &contatore, &medio, &massimo);
                    printf("\nDispositivo: %s\n", dispositivo);
                    printf("Numero richieste: %d\n", contatore);
                    printf("Costo medio stimato: %.2f EUR\n", medio);
                    printf("Costo massimo stimato: %.2f EUR\n", massimo);
                } else {
                    printf("Errore: tipologia non valida.\n");
                }
                break;
            case 2:
                statistichePerPriorita(richieste, numeroRichieste, conteggi);
                printf("\nConteggio per priorita:\n");
                printf("  BASSA: %d\n", conteggi[BASSA]);
                printf("  MEDIA: %d\n", conteggi[MEDIA]);
                printf("  ALTA:  %d\n", conteggi[ALTA]);
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Sottomenu salvataggio */
static void menuSalvataggio(const Richiesta *richieste, int numeroRichieste)
{
    int scelta;

    do {
        printf("\n--- SALVATAGGIO ---\n");
        printf("1. Salva richieste su file\n");
        printf("0. Torna al menu principale\n");

        if (!leggiIntero("Scelta: ", 0, 1, &scelta)) {
            printf("Errore: scelta non valida.\n");
            continue;
        }

        switch (scelta) {
            case 1:
                if (salvaRichieste(richieste, numeroRichieste) == 0) {
                    printf("Conferma: dati salvati in %s\n", FILE_RICHIESTE);
                } else {
                    printf("Errore: salvataggio non riuscito.\n");
                }
                break;
            case 0:
                break;
            default:
                break;
        }
    } while (scelta != 0);
}

/* Avvia il menu testuale principale e tutti i sottomenu */
void avviaInterfaccia(Richiesta *richieste, int *numeroRichieste)
{
    int scelta;

    if (richieste == NULL || numeroRichieste == NULL) {
        return;
    }

    mostraSchermataIniziale();
    printf("Richieste caricate: %d\n", *numeroRichieste);

    do {
        scelta = menuPrincipale();

        if (scelta == -1) {
            continue;
        }

        switch (scelta) {
            case 1:
                menuGestione(richieste, numeroRichieste);
                break;
            case 2:
                menuFiltri(richieste, *numeroRichieste);
                break;
            case 3:
                menuOrdinamenti(richieste, *numeroRichieste);
                break;
            case 4:
                menuReport(richieste, *numeroRichieste);
                break;
            case 5:
                menuStatistiche(richieste, *numeroRichieste);
                break;
            case 6:
                menuSalvataggio(richieste, *numeroRichieste);
                break;
            case 0:
                printf("\nUscita dal programma. Ricorda di salvare i dati se necessario.\n");
                break;
            default:
                break;
        }
    } while (scelta != 0);
}
