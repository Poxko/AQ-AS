#include "interfaccia.h"

int main(void)
{
    Richiesta richieste[MAX_RICHIESTE];
    int numeroRichieste = 0;

    caricaRichieste(richieste, &numeroRichieste);
    avviaInterfaccia(richieste, &numeroRichieste);

    return 0;
}
