#ifndef RICHIESTA_H
#define RICHIESTA_H

/* Capacità massima dell'array di richieste */
#define MAX_RICHIESTE 100

/* Dimensioni massime dei campi testuali */
#define MAX_CLIENTE 100
#define MAX_DISPOSITIVO 100
#define MAX_DESCRIZIONE 100
#define MAX_DATA 11

/* Valore sentinel per costo finale non ancora disponibile */
#define COSTO_FINALE_NON_DISPONIBILE (-1.0f)

/* Percorsi dei file di persistenza e report */
#define FILE_RICHIESTE "data/richieste.txt"
#define FILE_REPORT1 "data/report1_generale.txt"
#define FILE_REPORT2 "data/report2_operativo.txt"

/* Separatore dei campi nel file dati */
#define SEPARATORE_CAMPO '|'

/* Numero di valori negli enum (per statistiche e array di conteggio) */
#define NUM_PRIORITA 3
#define NUM_STATI 4

typedef enum {
    BASSA,
    MEDIA,
    ALTA
} Priorita;

typedef enum {
    APERTA,
    IN_LAVORAZIONE,
    COMPLETATA,
    ANNULLATA
} Stato;

typedef struct {
    int codice;
    char cliente[MAX_CLIENTE];
    char dispositivo[MAX_DISPOSITIVO];
    char descrizione[MAX_DESCRIZIONE];
    Priorita priorita;
    Stato stato;
    float costoStimato;
    float costoFinale;
    char dataApertura[MAX_DATA];
} Richiesta;

#endif /* RICHIESTA_H */
