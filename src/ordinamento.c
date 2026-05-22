#include "ordinamento.h"

#include <stdio.h>
#include <string.h>

/* Converte la data GG/MM/AAAA in un intero AAAAMMGG per confronti */
static int dataInNumero(const char *data)
{
    int giorno = 0;
    int mese = 0;
    int anno = 0;

    if (data == NULL || sscanf(data, "%d/%d/%d", &giorno, &mese, &anno) != 3) {
        return 0;
    }

    return anno * 10000 + mese * 100 + giorno;
}

/* Comparatore per ordinamento crescente per costo stimato */
static int confrontaCostoStimato(const Richiesta *a, const Richiesta *b)
{
    if (a->costoStimato < b->costoStimato) {
        return -1;
    }
    if (a->costoStimato > b->costoStimato) {
        return 1;
    }
    return 0;
}

/* Comparatore per ordinamento crescente per data di apertura */
static int confrontaDataApertura(const Richiesta *a, const Richiesta *b)
{
    int dataA = dataInNumero(a->dataApertura);
    int dataB = dataInNumero(b->dataApertura);

    if (dataA < dataB) {
        return -1;
    }
    if (dataA > dataB) {
        return 1;
    }
    return 0;
}

/* Scambia due elementi dell'array */
static void scambiaRichieste(Richiesta *a, Richiesta *b)
{
    Richiesta temporanea = *a;
    *a = *b;
    *b = temporanea;
}

/* Partizione per Quick Sort */
static int partizione(Richiesta *array, int basso, int alto, ComparatoreRichieste confronta)
{
    Richiesta pivot = array[alto];
    int i = basso - 1;
    int j;

    for (j = basso; j < alto; j++) {
        if (confronta(&array[j], &pivot) <= 0) {
            i++;
            scambiaRichieste(&array[i], &array[j]);
        }
    }

    scambiaRichieste(&array[i + 1], &array[alto]);
    return i + 1;
}

/* Quick Sort ricorsivo su un intervallo dell'array */
static void quickSort(Richiesta *array, int basso, int alto, ComparatoreRichieste confronta)
{
    int indicePartizione;

    if (basso < alto) {
        indicePartizione = partizione(array, basso, alto, confronta);
        quickSort(array, basso, indicePartizione - 1, confronta);
        quickSort(array, indicePartizione + 1, alto, confronta);
    }
}

/* Unisce due metà ordinate (fase del Merge Sort) */
static void merge(Richiesta *array, Richiesta *temporaneo, int sinistra, int centro, int destra,
                  ComparatoreRichieste confronta)
{
    int i = sinistra;
    int j = centro + 1;
    int k = sinistra;

    while (i <= centro && j <= destra) {
        if (confronta(&array[i], &array[j]) <= 0) {
            temporaneo[k++] = array[i++];
        } else {
            temporaneo[k++] = array[j++];
        }
    }

    while (i <= centro) {
        temporaneo[k++] = array[i++];
    }

    while (j <= destra) {
        temporaneo[k++] = array[j++];
    }

    for (i = sinistra; i <= destra; i++) {
        array[i] = temporaneo[i];
    }
}

/* Merge Sort ricorsivo su un intervallo dell'array */
static void mergeSort(Richiesta *array, Richiesta *temporaneo, int sinistra, int destra,
                    ComparatoreRichieste confronta)
{
    int centro;

    if (sinistra < destra) {
        centro = sinistra + (destra - sinistra) / 2;
        mergeSort(array, temporaneo, sinistra, centro, confronta);
        mergeSort(array, temporaneo, centro + 1, destra, confronta);
        merge(array, temporaneo, sinistra, centro, destra, confronta);
    }
}

/* Copia l'array sorgente in destinazione senza modificarlo */
void copiaArrayRichieste(const Richiesta *sorgente, Richiesta *destinazione, int numeroRichieste)
{
    int i;

    if (sorgente == NULL || destinazione == NULL || numeroRichieste <= 0) {
        return;
    }

    for (i = 0; i < numeroRichieste; i++) {
        destinazione[i] = sorgente[i];
    }
}

/* Ordina una copia per costo stimato crescente usando Quick Sort */
void ordinaPerCostoStimato(Richiesta *copia, int numeroRichieste)
{
    if (copia != NULL && numeroRichieste > 1) {
        quickSort(copia, 0, numeroRichieste - 1, confrontaCostoStimato);
    }
}

/* Ordina una copia per data di apertura crescente usando Merge Sort */
void ordinaPerDataApertura(Richiesta *copia, int numeroRichieste)
{
    Richiesta temporaneo[MAX_RICHIESTE];

    if (copia != NULL && numeroRichieste > 1) {
        mergeSort(copia, temporaneo, 0, numeroRichieste - 1, confrontaDataApertura);
    }
}
