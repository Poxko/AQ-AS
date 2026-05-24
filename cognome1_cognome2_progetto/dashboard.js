/* ===================================================================
 * dashboard.js
 * Logica della dashboard operativa — Centro Assistenza Tecnica
 * Compatibile con il programma C (formato pipe su richieste.txt)
 * =================================================================== */

/* ===== COSTANTI ===== */
const MAX_RICHIESTE      = 100;         /* numero massimo di richieste gestibili */
const COSTO_FINALE_ND    = -1.0;        /* sentinella: costo finale non ancora disponibile */
const DURATA_TOAST_MS    = 3200;        /* durata in millisecondi della notifica toast */
const NOME_FILE_RICHIESTE = "richieste.txt";
const NOME_FILE_REPORT1   = "report1_generale.txt";
const NOME_FILE_REPORT2   = "report2_operativo.txt";
const IDB_NOME            = "techfix-dashboard-db";
const IDB_STORE           = "impostazioni";
const IDB_CHIAVE_CARTELLA = "cartellaData";

/* costanti grafiche per il grafico a torta */
const SVG_CX            = 100;         /* centro X del grafico a torta */
const SVG_CY            = 100;         /* centro Y del grafico a torta */
const SVG_RAGGIO_TORTA  = 80;          /* raggio esterno del grafico a torta */
const SVG_RAGGIO_CENTRO = 35;          /* raggio del cerchio centrale (effetto donut) */

/* costanti grafiche per il grafico a barre */
const BARRA_H       = 24;              /* altezza di ciascuna barra in pixel */
const BARRA_GAP     = 8;               /* spazio verticale tra le barre in pixel */
const BARRA_MARGIN_L = 110;            /* margine sinistro delle barre in pixel */
const BARRA_MARGIN_T = 10;             /* margine superiore delle barre in pixel */
const BARRA_CHART_W  = 260;            /* larghezza massima delle barre in pixel */

/* ===== STATO GLOBALE ===== */
let cartellaDataHandle = null;          /* handle della cartella data/ collegata */

/* array principale delle richieste — aggiornato ad ogni operazione */
let richieste = [
    { codice: 1, cliente: "Mario Rossi",    dispositivo: "Smartphone", descrizione: "Schermo rotto",   priorita: "ALTA",  stato: "APERTA",    costoStimato: 80.00,   costoFinale: -1.00,  dataApertura: "15/05/2026" },
    { codice: 2, cliente: "Luca Bianchi",   dispositivo: "Notebook",   descrizione: "Non si accende", priorita: "MEDIA", stato: "ANNULLATA", costoStimato: 150.00,  costoFinale: 130.00, dataApertura: "10/05/2026" },
    { codice: 3, cliente: "Aurora Quarto",  dispositivo: "MacBook",    descrizione: "Fungeva",         priorita: "ALTA",  stato: "ANNULLATA", costoStimato: 1000.00, costoFinale: -1.00,  dataApertura: "22/05/2026" },
    { codice: 4, cliente: "Antonio Solare", dispositivo: "MacBook",    descrizione: "prova",           priorita: "ALTA",  stato: "APERTA",    costoStimato: 300.00,  costoFinale: -1.00,  dataApertura: "27/05/2026" }
];

let modalMode       = "nuova";          /* modalità del form: "nuova" | "modifica" */
let codiceInModifica = null;            /* codice della richiesta in modifica */

const STATI     = ["APERTA", "IN_LAVORAZIONE", "COMPLETATA", "ANNULLATA"];
const PRIORITA  = ["BASSA", "MEDIA", "ALTA"];

const COLORI_STATO = {
    APERTA:        "#3b82f6",
    IN_LAVORAZIONE:"#eab308",
    COMPLETATA:    "#10b981",
    ANNULLATA:     "#94a3b8"
};

const COLORI_DISPOSITIVO = [
    "#2563eb", "#7c3aed", "#db2777", "#ea580c",
    "#059669", "#0891b2", "#4f46e5", "#be123c"
];

/* ===== UTILITÀ GENERALI ===== */

/*
 * escapaHtml — converte caratteri speciali HTML in entità sicure.
 * Parametri: testo (string) — stringa da sanificare.
 * Ritorna:   stringa con caratteri HTML sostituiti da entità.
 */
function escapaHtml(testo) {
    const div = document.createElement("div");
    div.textContent = testo;
    return div.innerHTML;
}

/*
 * formattaDataOggi — restituisce la data odierna in formato GG/MM/AAAA.
 * Ritorna: stringa con la data corrente.
 */
function formattaDataOggi() {
    const oggi = new Date();
    return String(oggi.getDate()).padStart(2, "0") + "/" +
           String(oggi.getMonth() + 1).padStart(2, "0") + "/" +
           oggi.getFullYear();
}

/*
 * mostraToast — visualizza una notifica temporanea in sovrimpressione.
 * Parametri: msg     (string)  — testo da mostrare.
 *            isError (boolean) — se true mostra la notifica in rosso.
 */
function mostraToast(msg, isError) {
    const toast = document.getElementById("toast");
    toast.textContent = msg;
    toast.className = "toast show" + (isError ? " error" : "");
    setTimeout(function () { toast.classList.remove("show"); }, DURATA_TOAST_MS);
}

/*
 * isCostoFinaleDisponibile — verifica se il costo finale è stato impostato.
 * Parametri: v (number) — valore del costo finale.
 * Ritorna:   true se il costo finale è valido (diverso dalla sentinella -1).
 */
function isCostoFinaleDisponibile(v) {
    return (v >= 0) && (Math.abs(v - COSTO_FINALE_ND) > 0.005);
}

/*
 * classePriorita — restituisce la classe CSS corrispondente alla priorità.
 * Parametri: p (string) — valore della priorità (ALTA, MEDIA, BASSA).
 * Ritorna:   stringa con la classe CSS del badge.
 */
function classePriorita(p) {
    return "badge badge-" + p.toLowerCase();
}

/*
 * classeStato — restituisce la classe CSS corrispondente allo stato.
 * Parametri: s (string) — valore dello stato della richiesta.
 * Ritorna:   stringa con la classe CSS del badge.
 */
function classeStato(s) {
    const mappa = {
        APERTA:        "aperta",
        IN_LAVORAZIONE:"lavorazione",
        COMPLETATA:    "completata",
        ANNULLATA:     "annullata"
    };
    return "badge badge-" + (mappa[s] || "annullata");
}

/*
 * etichettaStato — restituisce l'etichetta leggibile dello stato.
 * Parametri: s (string) — valore dello stato della richiesta.
 * Ritorna:   stringa formattata per la visualizzazione all'utente.
 */
function etichettaStato(s) {
    if (s === "IN_LAVORAZIONE") {
        return "In lavorazione";
    }
    return s.charAt(0) + s.slice(1).toLowerCase();
}

/*
 * formattaCostoStimatoHtml — formatta il costo stimato per la tabella HTML.
 * Parametri: v (number) — valore del costo stimato.
 * Ritorna:   stringa HTML con il costo formattato in Euro.
 */
function formattaCostoStimatoHtml(v) {
    return escapaHtml(Number(v).toFixed(2)) + " €";
}

/*
 * formattaCostoFinaleHtml — formatta il costo finale per la tabella HTML.
 * Parametri: v (number) — valore del costo finale.
 * Ritorna:   stringa HTML con il costo in Euro, oppure badge "N/D".
 */
function formattaCostoFinaleHtml(v) {
    if (!isCostoFinaleDisponibile(v)) {
        return '<span class="costo-nd">N/D</span>';
    }
    return escapaHtml(Number(v).toFixed(2)) + " €";
}

/*
 * contaPerCampo — conta le occorrenze di ogni valore distinto in un campo.
 * Parametri: elenco (array)  — array di oggetti da analizzare.
 *            campo  (string) — nome del campo su cui raggruppare.
 * Ritorna:   oggetto { valore: conteggio } per ogni valore distinto.
 */
function contaPerCampo(elenco, campo) {
    const conteggi = {};
    elenco.forEach(function (r) {
        conteggi[r[campo]] = (conteggi[r[campo]] || 0) + 1;
    });
    return conteggi;
}

/*
 * generaNuovoCodice — calcola il prossimo codice univoco disponibile.
 * Ritorna: intero pari al massimo codice esistente + 1.
 */
function generaNuovoCodice() {
    let max = 0;
    richieste.forEach(function (r) {
        if (r.codice > max) {
            max = r.codice;
        }
    });
    return max + 1;
}

/*
 * trovaIndice — cerca una richiesta nell'array tramite codice.
 * Parametri: codice (number) — codice univoco della richiesta.
 * Ritorna:   indice nell'array, oppure -1 se non trovata.
 */
function trovaIndice(codice) {
    return richieste.findIndex(function (r) { return r.codice === codice; });
}

/*
 * isDataValida — verifica che una stringa rappresenti una data valida.
 * Parametri: data (string) — data nel formato GG/MM/AAAA.
 * Ritorna:   true se il formato e i valori sono corretti.
 */
function isDataValida(data) {
    if (!/^\d{2}\/\d{2}\/\d{4}$/.test(data)) {
        return false;
    }
    const parti = data.split("/");
    const giorno = parseInt(parti[0], 10);
    const mese   = parseInt(parti[1], 10);
    const anno   = parseInt(parti[2], 10);
    return (giorno >= 1) && (giorno <= 31) &&
           (mese >= 1)   && (mese <= 12)   &&
           (anno >= 1900);
}

/*
 * isStringaValida — verifica che una stringa non sia vuota o composta solo da spazi.
 * Parametri: s (string) — stringa da controllare.
 * Ritorna:   true se la stringa contiene almeno un carattere non spazio.
 */
function isStringaValida(s) {
    return s && s.trim().length > 0;
}

/* ===== SERIALIZZAZIONE richieste.txt ===== */

/*
 * rigaDaRichiesta — serializza una richiesta nel formato pipe del programma C.
 * Parametri: r (object) — oggetto richiesta da serializzare.
 * Ritorna:   stringa con i campi separati da '|'.
 */
function rigaDaRichiesta(r) {
    const costoFinale = isCostoFinaleDisponibile(r.costoFinale)
        ? r.costoFinale.toFixed(2)
        : (-1).toFixed(2);
    return [
        r.codice, r.cliente, r.dispositivo, r.descrizione,
        r.priorita, r.stato, r.costoStimato.toFixed(2),
        costoFinale, r.dataApertura
    ].join("|");
}

/*
 * esportaTestoRichieste — converte l'intero array in testo per richieste.txt.
 * Ritorna: stringa con tutte le richieste, una per riga.
 */
function esportaTestoRichieste() {
    return richieste.map(rigaDaRichiesta).join("\n") + (richieste.length ? "\n" : "");
}

/* ===== GESTIONE FILE SYSTEM LOCALE (File System Access API) ===== */

/*
 * isScritturaLocaleSupported — verifica se il browser supporta la scrittura su file.
 * Ritorna: true se l'API showDirectoryPicker è disponibile (Chrome/Edge).
 */
function isScritturaLocaleSupported() {
    return typeof window.showDirectoryPicker === "function";
}

/*
 * apriIndexedDB — apre (o crea) il database IndexedDB per persistere l'handle.
 * Ritorna: Promise che si risolve con l'oggetto IDBDatabase.
 */
function apriIndexedDB() {
    return new Promise(function (resolve, reject) {
        const richiesta = indexedDB.open(IDB_NOME, 1);
        richiesta.onupgradeneeded = function (e) {
            e.target.result.createObjectStore(IDB_STORE);
        };
        richiesta.onsuccess = function () { resolve(richiesta.result); };
        richiesta.onerror   = function () { reject(richiesta.error); };
    });
}

/*
 * salvaHandleInIndexedDB — persiste l'handle della cartella in IndexedDB.
 * Parametri: handle (FileSystemDirectoryHandle) — handle da salvare.
 * Ritorna:   Promise che si risolve al completamento.
 */
function salvaHandleInIndexedDB(handle) {
    return apriIndexedDB().then(function (db) {
        return new Promise(function (resolve, reject) {
            const tx = db.transaction(IDB_STORE, "readwrite");
            tx.objectStore(IDB_STORE).put(handle, IDB_CHIAVE_CARTELLA);
            tx.oncomplete = function () { resolve(); };
            tx.onerror    = function () { reject(tx.error); };
        });
    });
}

/*
 * caricaHandleDaIndexedDB — recupera l'handle della cartella da IndexedDB.
 * Ritorna: Promise che si risolve con l'handle, oppure null se assente.
 */
function caricaHandleDaIndexedDB() {
    return apriIndexedDB().then(function (db) {
        return new Promise(function (resolve, reject) {
            const tx  = db.transaction(IDB_STORE, "readonly");
            const get = tx.objectStore(IDB_STORE).get(IDB_CHIAVE_CARTELLA);
            get.onsuccess = function () { resolve(get.result || null); };
            get.onerror   = function () { reject(get.error); };
        });
    });
}

/*
 * verificaPermessoScrittura — controlla (e richiede se necessario) il permesso di scrittura.
 * Parametri: handle (FileSystemDirectoryHandle) — handle da verificare.
 * Ritorna:   Promise<boolean> — true se il permesso è concesso.
 */
async function verificaPermessoScrittura(handle) {
    if (!handle) {
        return false;
    }
    const opzioni = { mode: "readwrite" };
    if ((await handle.queryPermission(opzioni)) === "granted") {
        return true;
    }
    return (await handle.requestPermission(opzioni)) === "granted";
}

/*
 * aggiornaStatoCartellaData — aggiorna il messaggio di stato della cartella collegata.
 */
function aggiornaStatoCartellaData() {
    const elemento = document.getElementById("cartellaDataStatus");
    if (!isScritturaLocaleSupported()) {
        elemento.textContent = "Scrittura diretta non supportata: usa Chrome o Edge e collega la cartella data del progetto.";
        elemento.style.color = "#b91c1c";
        return;
    }
    if (cartellaDataHandle) {
        elemento.textContent = "Cartella collegata: data/ — i salvataggi aggiornano richieste.txt e i report esistenti.";
        elemento.style.color = "#047857";
    } else {
        elemento.textContent = "Clicca «Collega cartella data» e seleziona la cartella data del progetto (contiene richieste.txt).";
        elemento.style.color = "var(--text-muted)";
    }
}

/*
 * collegaCartellaData — apre il selettore di cartella e salva l'handle.
 * Ritorna: Promise<FileSystemDirectoryHandle> — handle della cartella selezionata.
 * Lancia:  Error se il browser non supporta l'API o il permesso è negato.
 */
async function collegaCartellaData() {
    if (!isScritturaLocaleSupported()) {
        throw new Error("Il browser non supporta la scrittura su file. Usa Chrome o Edge.");
    }
    const handle = await window.showDirectoryPicker({
        id:      "techfix-data",
        mode:    "readwrite",
        startIn: "documents"
    });
    if (!(await verificaPermessoScrittura(handle))) {
        throw new Error("Permesso di scrittura negato per la cartella selezionata.");
    }
    cartellaDataHandle = handle;
    await salvaHandleInIndexedDB(handle);
    aggiornaStatoCartellaData();
    return handle;
}

/*
 * ripristinaCartellaData — ripristina l'handle salvato in IndexedDB all'avvio.
 * Ritorna: Promise<FileSystemDirectoryHandle|null> — handle ripristinato, o null.
 */
async function ripristinaCartellaData() {
    if (!isScritturaLocaleSupported()) {
        return null;
    }
    try {
        const handle = await caricaHandleDaIndexedDB();
        if (handle && (await verificaPermessoScrittura(handle))) {
            cartellaDataHandle = handle;
            aggiornaStatoCartellaData();
            return handle;
        }
    } catch (e) { /* handle non disponibile o permesso revocato — ignorato */ }
    return null;
}

/*
 * ottieniCartellaData — restituisce l'handle attivo, richiedendo collegamento se assente.
 * Ritorna: Promise<FileSystemDirectoryHandle> — handle valido con permesso di scrittura.
 */
async function ottieniCartellaData() {
    if (cartellaDataHandle && (await verificaPermessoScrittura(cartellaDataHandle))) {
        return cartellaDataHandle;
    }
    return collegaCartellaData();
}

/*
 * scriviFileInData — sovrascrive un file nella cartella data/ collegata.
 * Parametri: nomeFile (string) — nome del file da scrivere.
 *            contenuto (string) — testo da scrivere nel file.
 */
async function scriviFileInData(nomeFile, contenuto) {
    const dir        = await ottieniCartellaData();
    const fileHandle = await dir.getFileHandle(nomeFile, { create: true });
    const writable   = await fileHandle.createWritable();
    await writable.write(contenuto);
    await writable.close();
}

/*
 * caricaRichiesteDaCartellaData — legge richieste.txt dalla cartella collegata.
 * Aggiorna l'array richieste e ridisegna la dashboard.
 * Lancia: Error se il file non contiene righe valide.
 */
async function caricaRichiesteDaCartellaData() {
    const dir        = await ottieniCartellaData();
    const fileHandle = await dir.getFileHandle(NOME_FILE_RICHIESTE);
    const file       = await fileHandle.getFile();
    const parsate    = parseFileRichieste(await file.text());
    if (!parsate.length) {
        throw new Error("Il file richieste.txt non contiene righe valide.");
    }
    richieste = parsate;
    document.getElementById("filtroStato").value       = "";
    document.getElementById("filtroPriorita").value    = "";
    document.getElementById("filtroDispositivo").value = "";
    popolaFiltroDispositivi();
    aggiornaDashboard();
    document.getElementById("syncStatus").textContent =
        "Caricate " + parsate.length + " richieste da data/richieste.txt";
}

/* ===== GENERAZIONE REPORT ===== */

/*
 * scriviRigaReport — formatta una riga di dettaglio per i file di report.
 * Parametri: r (object) — oggetto richiesta da formattare.
 * Ritorna:   stringa con tutti i campi separati da ' | '.
 */
function scriviRigaReport(r) {
    let riga = "  [" + r.codice + "] " + r.cliente + " | " + r.dispositivo + " | " +
               r.descrizione + " | " + r.priorita + " | " + r.stato +
               " | stimato: " + r.costoStimato.toFixed(2);
    if (isCostoFinaleDisponibile(r.costoFinale)) {
        riga += " | finale: " + r.costoFinale.toFixed(2);
    } else {
        riga += " | finale: N/D";
    }
    return riga + " | " + r.dataApertura + "\n";
}

/*
 * quickSort — ordina un array tramite l'algoritmo Quick Sort.
 * Parametri: arr (array)    — array da ordinare (non modificato: lavora su copia).
 *            cmp (function) — funzione comparatore: cmp(a, b) < 0 se a < b.
 * Ritorna:   nuovo array ordinato.
 */
function quickSort(arr, cmp) {
    if (arr.length <= 1) {
        return arr;
    }
    const pivot = arr[arr.length - 1];
    const minori = [], maggiori = [], uguali = [];
    arr.slice(0, -1).forEach(function (x) {
        const risultato = cmp(x, pivot);
        if (risultato < 0) {
            minori.push(x);
        } else if (risultato > 0) {
            maggiori.push(x);
        } else {
            uguali.push(x);
        }
    });
    return quickSort(minori, cmp).concat(uguali, [pivot], quickSort(maggiori, cmp));
}

/*
 * generaReport1 — produce il contenuto testuale del Report Generale.
 * Ritorna: stringa con statistiche totali, per stato, per priorità e costi medi.
 */
function generaReport1() {
    let testo = "========================================\n";
    testo += "       REPORT 1 - GENERALE\n";
    testo += "========================================\n\n";
    testo += "Totale richieste: " + richieste.length + "\n\n";

    testo += "Richieste per stato:\n";
    STATI.forEach(function (s) {
        const n = richieste.filter(function (r) { return r.stato === s; }).length;
        testo += "  - " + s + ": " + n + "\n";
    });

    testo += "\nRichieste per priorita:\n";
    PRIORITA.forEach(function (p) {
        const n = richieste.filter(function (r) { return r.priorita === p; }).length;
        testo += "  - " + p + ": " + n + "\n";
    });

    /* calcolo costo medio stimato su tutte le richieste */
    let sommaStimato = 0;
    richieste.forEach(function (r) { sommaStimato += r.costoStimato; });
    const mediaStiamto = richieste.length ? sommaStimato / richieste.length : 0;

    /* calcolo costo medio finale sulle sole richieste completate */
    const completate = richieste.filter(function (r) {
        return (r.stato === "COMPLETATA") && isCostoFinaleDisponibile(r.costoFinale);
    });
    let sommaFinale = 0;
    completate.forEach(function (r) { sommaFinale += r.costoFinale; });
    const mediaFinale = completate.length ? sommaFinale / completate.length : 0;

    testo += "\nCosto medio stimato (tutte): " + mediaStiamto.toFixed(2) + " EUR\n";
    testo += "Costo medio finale (completate): " + mediaFinale.toFixed(2) + " EUR\n\n";

    /* elenco ordinato per costo stimato crescente tramite Quick Sort */
    const ordinate = quickSort(richieste.slice(), function (a, b) {
        return a.costoStimato - b.costoStimato;
    });
    testo += "Elenco ordinato per costo stimato (crescente):\n";
    if (ordinate.length === 0) {
        testo += "Nessuna richiesta da elencare.\n";
    } else {
        ordinate.forEach(function (r) { testo += scriviRigaReport(r); });
    }
    return testo;
}

/*
 * statisticheDispositivo — calcola statistiche di costo per una tipologia di dispositivo.
 * Parametri: nome (string) — nome del dispositivo da analizzare.
 * Ritorna:   oggetto { contatore, medio, max } con le statistiche calcolate.
 */
function statisticheDispositivo(nome) {
    const filtrate = richieste.filter(function (r) {
        return r.dispositivo.toLowerCase() === nome.toLowerCase();
    });
    if (!filtrate.length) {
        return { contatore: 0, medio: 0, max: 0 };
    }
    let somma = 0;
    let massimo = 0;
    filtrate.forEach(function (r) {
        somma += r.costoStimato;
        if (r.costoStimato > massimo) {
            massimo = r.costoStimato;
        }
    });
    return { contatore: filtrate.length, medio: somma / filtrate.length, max: massimo };
}

/*
 * generaReport2 — produce il contenuto testuale del Report Operativo.
 * Ritorna: stringa con elenchi per stato (alta priorità aperte, in lavorazione,
 *          completate, annullate) e riepilogo per tipologia dispositivo.
 */
function generaReport2() {
    let testo = "========================================\n";
    testo += "       REPORT 2 - OPERATIVO\n";
    testo += "========================================\n\n";

    /* richieste ad alta priorità ancora aperte */
    testo += ">>> Richieste ALTA priorita ancora APERTE:\n";
    const altaAperte = richieste.filter(function (r) {
        return (r.priorita === "ALTA") && (r.stato === "APERTA");
    });
    if (!altaAperte.length) {
        testo += "  (nessuna)\n";
    } else {
        altaAperte.forEach(function (r) { testo += scriviRigaReport(r); });
    }
    testo += "\n";

    /* elenchi per i restanti stati */
    const titoli = {
        IN_LAVORAZIONE: "IN LAVORAZIONE",
        COMPLETATA:     "COMPLETATE",
        ANNULLATA:      "ANNULLATE"
    };
    ["IN_LAVORAZIONE", "COMPLETATA", "ANNULLATA"].forEach(function (stato) {
        testo += ">>> Richieste " + titoli[stato] + ":\n";
        const lista = richieste.filter(function (r) { return r.stato === stato; });
        if (!lista.length) {
            testo += "  (nessuna)\n";
        } else {
            lista.forEach(function (r) { testo += scriviRigaReport(r); });
        }
        testo += "\n";
    });

    /* riepilogo statistiche per tipologia dispositivo */
    testo += ">>> Riepilogo per tipologia dispositivo:\n";
    const tipi = [];
    richieste.forEach(function (r) {
        if (!tipi.some(function (t) { return t.toLowerCase() === r.dispositivo.toLowerCase(); })) {
            tipi.push(r.dispositivo);
        }
    });
    tipi.sort();
    if (!tipi.length) {
        testo += "  (nessun dispositivo)\n";
    } else {
        tipi.forEach(function (nome) {
            const s = statisticheDispositivo(nome);
            testo += "  " + nome +
                     " -> richieste: " + s.contatore +
                     " | costo medio stimato: " + s.medio.toFixed(2) + " EUR" +
                     " | costo max stimato: " + s.max.toFixed(2) + " EUR\n";
        });
    }
    return testo;
}

/* ===== GRAFICI E CARDS ===== */

/*
 * aggiornaCards — aggiorna i contatori nelle card statistiche in cima alla pagina.
 * Parametri: dati (array) — array di richieste da contare.
 */
function aggiornaCards(dati) {
    const conteggi = contaPerCampo(dati, "stato");
    document.getElementById("statTotale").textContent     = dati.length;
    document.getElementById("statAperte").textContent     = conteggi["APERTA"]        || 0;
    document.getElementById("statLavorazione").textContent = conteggi["IN_LAVORAZIONE"] || 0;
    document.getElementById("statCompletate").textContent  = conteggi["COMPLETATA"]    || 0;
}

/*
 * disegnaGraficoStato — disegna il grafico a torta della distribuzione per stato.
 * Parametri: dati (array) — array di richieste da rappresentare.
 */
function disegnaGraficoStato(dati) {
    const svg    = document.getElementById("chartStato");
    const legend = document.getElementById("legendStato");
    const conteggi = contaPerCampo(dati, "stato");
    const chiavi  = Object.keys(conteggi);
    const totale  = dati.length || 1;
    let pathSvg     = "";
    let angoloStart = -Math.PI / 2;

    svg.innerHTML    = "";
    legend.innerHTML = "";

    chiavi.forEach(function (stato) {
        const val      = conteggi[stato];
        const frazione = val / totale;
        const angoloEnd = angoloStart + (frazione * 2 * Math.PI);
        const x1 = SVG_CX + SVG_RAGGIO_TORTA * Math.cos(angoloStart);
        const y1 = SVG_CY + SVG_RAGGIO_TORTA * Math.sin(angoloStart);
        const x2 = SVG_CX + SVG_RAGGIO_TORTA * Math.cos(angoloEnd);
        const y2 = SVG_CY + SVG_RAGGIO_TORTA * Math.sin(angoloEnd);
        const colore   = COLORI_STATO[stato] || "#64748b";
        const arcLarge = frazione > 0.5 ? 1 : 0;

        pathSvg += '<path d="M ' + SVG_CX + ' ' + SVG_CY +
                   ' L ' + x1 + ' ' + y1 +
                   ' A ' + SVG_RAGGIO_TORTA + ' ' + SVG_RAGGIO_TORTA +
                   ' 0 ' + arcLarge + ' 1 ' + x2 + ' ' + y2 +
                   ' Z" fill="' + colore + '" stroke="#fff" stroke-width="2"/>';
        angoloStart = angoloEnd;

        const voce = document.createElement("li");
        voce.innerHTML = '<span class="legend-dot" style="background:' + colore + '"></span>' +
                         etichettaStato(stato) + ': <strong>' + val + '</strong>';
        legend.appendChild(voce);
    });

    if (!chiavi.length) {
        svg.innerHTML = '<circle cx="' + SVG_CX + '" cy="' + SVG_CY +
                        '" r="' + SVG_RAGGIO_TORTA + '" fill="#e2e8f0"/>';
    } else {
        svg.innerHTML = pathSvg +
                        '<circle cx="' + SVG_CX + '" cy="' + SVG_CY +
                        '" r="' + SVG_RAGGIO_CENTRO + '" fill="#fff"/>';
    }
}

/*
 * disegnaGraficoDispositivo — disegna il grafico a barre orizzontali per dispositivo.
 * Parametri: dati (array) — array di richieste da rappresentare.
 */
function disegnaGraficoDispositivo(dati) {
    const svg      = document.getElementById("chartDispositivo");
    const conteggi = contaPerCampo(dati, "dispositivo");
    const chiavi   = Object.keys(conteggi).sort(function (a, b) {
        return conteggi[b] - conteggi[a];
    });
    const massimo  = Math.max.apply(null, chiavi.map(function (k) { return conteggi[k]; })) || 1;
    const altezza  = BARRA_MARGIN_T + (chiavi.length * (BARRA_H + BARRA_GAP)) + 10;

    svg.setAttribute("viewBox", "0 0 400 " + Math.max(220, altezza));

    let contenuto = "";
    chiavi.forEach(function (nome, i) {
        const val      = conteggi[nome];
        const y        = BARRA_MARGIN_T + (i * (BARRA_H + BARRA_GAP));
        const larghezza = (val / massimo) * BARRA_CHART_W;
        const colore   = COLORI_DISPOSITIVO[i % COLORI_DISPOSITIVO.length];
        const yTesto   = y + (BARRA_H / 2) + 4;

        contenuto += '<text x="8" y="' + yTesto + '" font-size="11" fill="#334155">' +
                     escapaHtml(nome) + '</text>';
        contenuto += '<rect x="' + BARRA_MARGIN_L + '" y="' + y + '" width="' + larghezza +
                     '" height="' + BARRA_H + '" rx="4" fill="' + colore + '"/>';
        contenuto += '<text x="' + (BARRA_MARGIN_L + larghezza + 6) + '" y="' + yTesto +
                     '" font-size="11" fill="#64748b">' + val + '</text>';
    });

    svg.innerHTML = contenuto || '<text x="20" y="100" fill="#94a3b8" font-size="14">Nessun dato</text>';
}

/* ===== PARSING FILE ===== */

/*
 * parseRigaRichiesta — analizza una singola riga del file richieste.txt.
 * Parametri: linea (string) — riga da analizzare nel formato pipe.
 * Ritorna:   oggetto richiesta, oppure null se il formato non è valido.
 */
function parseRigaRichiesta(linea) {
    const parti = linea.trim().split("|");
    if (parti.length !== 9) {
        return null;
    }
    return {
        codice:       parseInt(parti[0], 10),
        cliente:      parti[1],
        dispositivo:  parti[2],
        descrizione:  parti[3],
        priorita:     parti[4],
        stato:        parti[5],
        costoStimato: parseFloat(parti[6]),
        costoFinale:  parseFloat(parti[7]),
        dataApertura: parti[8]
    };
}

/*
 * parseFileRichieste — analizza l'intero contenuto di richieste.txt.
 * Parametri: contenuto (string) — testo del file da analizzare.
 * Ritorna:   array di oggetti richiesta validi.
 */
function parseFileRichieste(contenuto) {
    const risultato = [];
    contenuto.split(/\r?\n/).forEach(function (riga) {
        if (!riga.trim()) {
            return;
        }
        const rec = parseRigaRichiesta(riga);
        if (rec && !isNaN(rec.codice)) {
            risultato.push(rec);
        }
    });
    return risultato;
}

/*
 * popolaFiltroDispositivi — aggiorna il menu a tendina dei dispositivi disponibili.
 * Mantiene la selezione corrente se ancora valida.
 */
function popolaFiltroDispositivi() {
    const select      = document.getElementById("filtroDispositivo");
    const valoreAttuale = select.value;

    /* rimuove tutte le opzioni tranne la prima ("Tutti") */
    while (select.options.length > 1) {
        select.remove(1);
    }

    /* costruisce l'elenco dei dispositivi unici presenti */
    const tipi = [];
    richieste.forEach(function (r) {
        if (!tipi.includes(r.dispositivo)) {
            tipi.push(r.dispositivo);
        }
    });
    tipi.sort();

    tipi.forEach(function (d) {
        const opzione       = document.createElement("option");
        opzione.value       = d;
        opzione.textContent = d;
        select.appendChild(opzione);
    });

    select.value = valoreAttuale;
}

/* ===== FILTRI E TABELLA ===== */

/*
 * filtraDati — filtra le richieste in base ai criteri selezionati nei menu.
 * Ritorna: array con le sole richieste che soddisfano tutti i filtri attivi.
 */
function filtraDati() {
    const stato      = document.getElementById("filtroStato").value;
    const priorita   = document.getElementById("filtroPriorita").value;
    const dispositivo = document.getElementById("filtroDispositivo").value;
    return richieste.filter(function (r) {
        if (stato       && (r.stato       !== stato))       { return false; }
        if (priorita    && (r.priorita    !== priorita))    { return false; }
        if (dispositivo && (r.dispositivo !== dispositivo)) { return false; }
        return true;
    });
}

/*
 * renderTabella — popola la tabella HTML con le richieste filtrate.
 * Parametri: dati (array) — array di richieste da visualizzare.
 */
function renderTabella(dati) {
    const corpo = document.getElementById("corpoTabella");
    const vuoto = document.getElementById("messaggioVuoto");
    corpo.innerHTML = "";

    if (!dati.length) {
        vuoto.hidden = false;
        return;
    }
    vuoto.hidden = true;

    dati.forEach(function (r) {
        const riga      = document.createElement("tr");
        const annullata = (r.stato === "ANNULLATA");
        riga.innerHTML =
            "<td>" + r.codice + "</td>" +
            "<td>" + escapaHtml(r.cliente) + "</td>" +
            "<td>" + escapaHtml(r.dispositivo) + "</td>" +
            "<td>" + escapaHtml(r.descrizione) + "</td>" +
            '<td><span class="' + classePriorita(r.priorita) + '">' + r.priorita + "</span></td>" +
            '<td><span class="' + classeStato(r.stato) + '">' + etichettaStato(r.stato) + "</span></td>" +
            "<td>" + formattaCostoStimatoHtml(r.costoStimato) + "</td>" +
            "<td>" + formattaCostoFinaleHtml(r.costoFinale) + "</td>" +
            "<td>" + escapaHtml(r.dataApertura) + "</td>" +
            '<td class="actions-cell">' +
            '<button type="button" class="btn btn-secondary btn-sm btn-modifica" data-codice="' + r.codice + '">Modifica</button>' +
            '<button type="button" class="btn btn-danger btn-sm btn-annulla" data-codice="' + r.codice + '"' +
            (annullata ? " disabled" : "") + '>Annulla</button></td>';
        corpo.appendChild(riga);
    });
}

/*
 * aggiornaDashboard — ridisegna l'intera dashboard (cards, grafici, tabella).
 */
function aggiornaDashboard() {
    const datiFiltrati = filtraDati();
    aggiornaCards(richieste);
    disegnaGraficoStato(richieste);
    disegnaGraficoDispositivo(richieste);
    renderTabella(datiFiltrati);
}

/* ===== GESTIONE MODALE FORM ===== */

/*
 * impostaCampiModifica — abilita o disabilita i campi non modificabili in modalità modifica.
 * Parametri: soloModifica (boolean) — se true blocca i campi fissi (cliente, dispositivo, ecc.).
 */
function impostaCampiModifica(soloModifica) {
    const campiBloccati = ["fldCliente", "fldDispositivo", "fldPriorita", "fldData"];
    campiBloccati.forEach(function (id) {
        document.getElementById(id).disabled = soloModifica;
    });
    document.getElementById("wrapCostoFinale").classList.add("form-field--hidden");
}

/*
 * apriModalNuova — apre il form in modalità inserimento nuova richiesta.
 */
function apriModalNuova() {
    modalMode        = "nuova";
    codiceInModifica = null;
    document.getElementById("modalTitolo").textContent = "Nuova richiesta";
    impostaCampiModifica(false);
    document.getElementById("fldCodice").value       = generaNuovoCodice();
    document.getElementById("fldCliente").value      = "";
    document.getElementById("fldDispositivo").value  = "";
    document.getElementById("fldDescrizione").value  = "";
    document.getElementById("fldPriorita").value     = "MEDIA";
    document.getElementById("fldStato").value        = "APERTA";
    document.getElementById("fldCostoStimato").value = "";
    document.getElementById("fldData").value         = formattaDataOggi();
    document.getElementById("formErrore").classList.remove("visible");
    document.getElementById("modalOverlay").classList.add("active");
}

/*
 * apriModalModifica — apre il form precompilato per la modifica di una richiesta.
 * Parametri: codice (number) — codice della richiesta da modificare.
 */
function apriModalModifica(codice) {
    const idx = trovaIndice(codice);
    if (idx === -1) {
        return;
    }
    const r = richieste[idx];
    if (r.stato === "ANNULLATA") {
        mostraToast("Impossibile modificare una richiesta annullata.", true);
        return;
    }
    modalMode        = "modifica";
    codiceInModifica = codice;
    document.getElementById("modalTitolo").textContent = "Modifica richiesta #" + codice;
    impostaCampiModifica(true);
    document.getElementById("fldCodice").value       = r.codice;
    document.getElementById("fldCliente").value      = r.cliente;
    document.getElementById("fldDispositivo").value  = r.dispositivo;
    document.getElementById("fldDescrizione").value  = r.descrizione;
    document.getElementById("fldPriorita").value     = r.priorita;
    document.getElementById("fldStato").value        = r.stato;
    document.getElementById("fldCostoStimato").value = r.costoStimato;
    document.getElementById("fldData").value         = r.dataApertura;
    document.getElementById("formErrore").classList.remove("visible");
    document.getElementById("modalOverlay").classList.add("active");
}

/*
 * chiudiModal — chiude il form modale senza salvare.
 */
function chiudiModal() {
    document.getElementById("modalOverlay").classList.remove("active");
}

/*
 * salvaForm — valida i dati del form e salva la richiesta (nuova o modificata).
 */
function salvaForm() {
    const err          = document.getElementById("formErrore");
    const descrizione  = document.getElementById("fldDescrizione").value.trim();
    const stato        = document.getElementById("fldStato").value;
    const costoStimato = parseFloat(document.getElementById("fldCostoStimato").value);

    if (modalMode === "nuova") {
        const cliente     = document.getElementById("fldCliente").value.trim();
        const dispositivo = document.getElementById("fldDispositivo").value.trim();
        const priorita    = document.getElementById("fldPriorita").value;
        const data        = document.getElementById("fldData").value.trim();

        if (!isStringaValida(cliente) || !isStringaValida(dispositivo) || !isStringaValida(descrizione)) {
            err.textContent = "Cliente, dispositivo e descrizione non possono essere vuoti.";
            err.classList.add("visible");
            return;
        }
        if (isNaN(costoStimato) || (costoStimato < 0)) {
            err.textContent = "Il costo stimato deve essere un numero >= 0.";
            err.classList.add("visible");
            return;
        }
        if (!isDataValida(data)) {
            err.textContent = "Data non valida. Usa il formato GG/MM/AAAA.";
            err.classList.add("visible");
            return;
        }
        if (richieste.length >= MAX_RICHIESTE) {
            err.textContent = "Limite massimo di " + MAX_RICHIESTE + " richieste raggiunto.";
            err.classList.add("visible");
            return;
        }
        richieste.push({
            codice:       generaNuovoCodice(),
            cliente:      cliente,
            dispositivo:  dispositivo,
            descrizione:  descrizione,
            priorita:     priorita,
            stato:        stato,
            costoStimato: costoStimato,
            costoFinale:  COSTO_FINALE_ND,
            dataApertura: data
        });
        mostraToast("Richiesta inserita con successo.");
    } else {
        const idx = trovaIndice(codiceInModifica);
        if (idx === -1) {
            return;
        }
        if (!isStringaValida(descrizione)) {
            err.textContent = "La descrizione non può essere vuota.";
            err.classList.add("visible");
            return;
        }
        if (isNaN(costoStimato) || (costoStimato < 0)) {
            err.textContent = "Il costo stimato deve essere >= 0.";
            err.classList.add("visible");
            return;
        }
        richieste[idx].stato        = stato;
        richieste[idx].descrizione  = descrizione;
        richieste[idx].costoStimato = costoStimato;
        mostraToast("Richiesta #" + codiceInModifica + " aggiornata.");
    }

    err.classList.remove("visible");
    chiudiModal();
    popolaFiltroDispositivi();
    aggiornaDashboard();
}

/*
 * eseguiAnnullamento — imposta lo stato di una richiesta ad ANNULLATA.
 * Parametri: codice (number) — codice della richiesta da annullare.
 */
function eseguiAnnullamento(codice) {
    const idx = trovaIndice(codice);
    if (idx === -1) {
        return;
    }
    if (richieste[idx].stato === "ANNULLATA") {
        mostraToast("La richiesta è già annullata.", true);
        return;
    }
    if (!confirm("Confermi l'annullamento della richiesta #" + codice + "?")) {
        return;
    }
    richieste[idx].stato = "ANNULLATA";
    mostraToast("Richiesta #" + codice + " annullata.");
    aggiornaDashboard();
}

/* ===== REGISTRAZIONE EVENTI ===== */

document.getElementById("dataCorrente").textContent = formattaDataOggi();
popolaFiltroDispositivi();

document.getElementById("btnNuova").addEventListener("click", apriModalNuova);
document.getElementById("btnChiudiModal").addEventListener("click", chiudiModal);
document.getElementById("btnAnnullaForm").addEventListener("click", chiudiModal);
document.getElementById("btnConfermaForm").addEventListener("click", salvaForm);

document.getElementById("modalOverlay").addEventListener("click", function (e) {
    if (e.target === document.getElementById("modalOverlay")) {
        chiudiModal();
    }
});

document.getElementById("btnCollegaData").addEventListener("click", async function () {
    try {
        await collegaCartellaData();
        await caricaRichiesteDaCartellaData();
        mostraToast("Cartella data collegata e richieste.txt caricato.");
    } catch (e) {
        mostraToast(e.message || "Collegamento annullato.", true);
    }
});

document.getElementById("btnSalva").addEventListener("click", async function () {
    try {
        await scriviFileInData(NOME_FILE_RICHIESTE, esportaTestoRichieste());
        mostraToast("Aggiornato: data/richieste.txt");
    } catch (e) {
        mostraToast(e.message || "Salvataggio non riuscito.", true);
    }
});

document.getElementById("btnReport1").addEventListener("click", async function () {
    try {
        await scriviFileInData(NOME_FILE_REPORT1, generaReport1());
        mostraToast("Aggiornato: data/report1_generale.txt");
    } catch (e) {
        mostraToast(e.message || "Report 1 non aggiornato.", true);
    }
});

document.getElementById("btnReport2").addEventListener("click", async function () {
    try {
        await scriviFileInData(NOME_FILE_REPORT2, generaReport2());
        mostraToast("Aggiornato: data/report2_operativo.txt");
    } catch (e) {
        mostraToast(e.message || "Report 2 non aggiornato.", true);
    }
});

document.getElementById("filtroStato").addEventListener("change", aggiornaDashboard);
document.getElementById("filtroPriorita").addEventListener("change", aggiornaDashboard);
document.getElementById("filtroDispositivo").addEventListener("change", aggiornaDashboard);

document.getElementById("inputRichiesteTxt").addEventListener("change", function (e) {
    if (!e.target.files || !e.target.files[0]) {
        return;
    }
    const reader = new FileReader();
    reader.onload = function (ev) {
        const parsate = parseFileRichieste(ev.target.result);
        if (!parsate.length) {
            mostraToast("Nessuna riga valida nel file.", true);
            return;
        }
        richieste = parsate;
        document.getElementById("filtroStato").value       = "";
        document.getElementById("filtroPriorita").value    = "";
        document.getElementById("filtroDispositivo").value = "";
        popolaFiltroDispositivi();
        aggiornaDashboard();
        document.getElementById("syncStatus").textContent =
            "Caricate " + parsate.length + " richieste da file.";
        mostraToast("Dati caricati da richieste.txt");
    };
    reader.readAsText(e.target.files[0], "UTF-8");
});

document.getElementById("corpoTabella").addEventListener("click", function (e) {
    const btn = e.target.closest("button");
    if (!btn) {
        return;
    }
    const codice = parseInt(btn.getAttribute("data-codice"), 10);
    if (btn.classList.contains("btn-modifica")) {
        apriModalModifica(codice);
    }
    if (btn.classList.contains("btn-annulla")) {
        eseguiAnnullamento(codice);
    }
});

document.getElementById("syncStatus").textContent =
    richieste.length + " richieste in memoria (embedded). Collega data/ per sincronizzare con il programma C.";

/* avvio: ripristina la cartella collegata in sessione precedente, se disponibile */
aggiornaStatoCartellaData();
ripristinaCartellaData().then(function (handle) {
    if (handle) {
        return caricaRichiesteDaCartellaData().then(function () {
            mostraToast("Ripristinata cartella data e caricato richieste.txt");
        }).catch(function () { /* file assente o vuoto — ignorato */ });
    }
});

aggiornaDashboard();