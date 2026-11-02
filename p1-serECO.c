/**************************************************************************/
/*                                                                        */
/* L'aplicació ECO amb sockets TCP/IP                                     */
/* Fitxer serECO.c que implementa el servidor ECO sobre la capa de        */
/* transport TCP (fent crides a la "nova" interfície de la capa de        */
/* transport o "nova" interfície de sockets).                             */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "p1-tTCP.h"

#define MAX_BUFFER 200

int Sck = -1;
int scon = -1;

/* Declaració de funcions INTERNES */
void tancament_suau(int sig);

/* Ús: serECO [@IP_on_escoltar]                                           */
/*  - sense argument s'escolta a 0.0.0.0 (MS1, casos 2 i 3)               */
/*  - amb l'@IP d'eth1 com a argument (p.e. ./serECO 82.2.3.4) es fa el   */
/*    bind a una @IP concreta (MS1, cas 1)                                */
int main(int argc, char *argv[])
{
    char IPesc[16] = "0.0.0.0";
    char IPrem[16];
    int portTCPrem;
    char IPloc[16];
    int portTCPloc;
    char buffer[MAX_BUFFER];
    int bytesEnviats, bytesRebuts;
    int continuar = 1;
    char msg[150];
    int len;

    if (argc > 1) {
        strncpy(IPesc, argv[1], 15);
        IPesc[15] = '\0';
    }

    /* MS4: quan l'administrador premi ^C s'executarà tancament_suau()     */
    signal(SIGINT, tancament_suau);

    // Creem el socket servidor. Posant el port 0, el SO n'assignarà un de lliure.
    Sck = TCP_CreaSockServidor(IPesc, 0);
    if (Sck < 0) {
        int codiRes;
		char* Missatge = T_ObteTextRes(&codiRes);
		printf("Error en socket() -> errno: %d \n Missatge: %s\n", codiRes, Missatge);
        return -1;
    }

    // Com que hem usat el port 0, mirem quin port ens han assignat per saber on connectar el client
    if (TCP_TrobaAdrSockLoc(Sck, IPloc, &portTCPloc) == 0) {
        len = snprintf(msg, sizeof(msg), "Servidor escoltant al port %d\n", portTCPloc);
        write(1, msg, len);
    }

    while (continuar) {
        scon = TCP_AcceptaConnexio(Sck, IPrem, &portTCPrem);
        if (scon < 0) {
            perror("Error en connectar amb el client");
        }
        else {
            /* MS1: sesc i scon són 2 descriptors del mateix socket (mateixa @) */

            // 1. Adreça (getsockname) del descriptor d'escolta (sesc)
            if (TCP_TrobaAdrSockLoc(Sck, IPloc, &portTCPloc) == 0) {
                len = snprintf(msg, sizeof(msg), "[MS1] sesc -> @IP: %s, #port TCP: %d\n", IPloc, portTCPloc);
                write(1, msg, len);
            }

            // 2. Adreça (getsockname) del descriptor de la connexió (scon)
            if (TCP_TrobaAdrSockLoc(scon, IPloc, &portTCPloc) == 0) {
                len = snprintf(msg, sizeof(msg), "[MS1] scon -> @IP: %s, #port TCP: %d\n", IPloc, portTCPloc);
                write(1, msg, len);
            }

            // 3. Adreça del socket remot (el client)
            len = snprintf(msg, sizeof(msg), "Socket remot: @IP: %s, #port TCP: %d\n", IPrem, portTCPrem);
            write(1, msg, len);

            int rep = 1;
            while (rep) {
                bytesRebuts = TCP_Rep(scon, buffer, MAX_BUFFER - 1);
                if (bytesRebuts < 0) {
                    perror("Error rebre dades");
                    rep = 0;
                }
                else if (bytesRebuts == 0) {
                    write(1, "Connexió tancada pel client.\n", 30);
                    rep = 0;
                }
                else {
                    buffer[bytesRebuts] = '\0';
                    write(1, "Rebut: ", 7);
                    write(1, buffer, strlen(buffer));
                    write(1, "\n", 1);

                    bytesEnviats = TCP_Envia(scon, buffer, bytesRebuts);
                    if (bytesEnviats < 0) {
                        perror("Error en enviar l'eco");
                        rep = 0;
                        continuar = 0;
                    }
                }
            }
            TCP_TancaSock(scon);
            scon = -1;   /* ja no hi ha connexió oberta */
        }
    }
    TCP_TancaSock(Sck);
    return 0;
}

/* Definició de funcions INTERNES */

/* MS4: gestor de SIGINT (^C). Tanca els sockets oberts        */

void tancament_suau(int sig)
{
    write(1, "\n[MS4] Senyal Ctrl+C rebut. Tancant suaument...\n", 48);
    if (scon >= 0) {
        TCP_TancaSock(scon);
    }
    if (Sck >= 0) {
        TCP_TancaSock(Sck);
    }
    write(1, "Sockets alliberats. Servidor apagat.\n", 37);
    exit(0);
}
