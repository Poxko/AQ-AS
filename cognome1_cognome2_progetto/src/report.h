#ifndef REPORT_H
#define REPORT_H

#include "richiesta.h"

/* Genera il report generale su FILE_REPORT1 */
int generaReportGenerale(const Richiesta *richieste, int numeroRichieste);

/* Genera il report operativo su FILE_REPORT2 */
int generaReportOperativo(const Richiesta *richieste, int numeroRichieste);

#endif /* REPORT_H */
