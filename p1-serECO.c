#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "p1-tTCP.h"

#define MAX_BUFFER 200

int main(int argc, char *argv[])
{
    char IPrem[16];
    int portTCPrem;
    char IPloc[16];
    int portTCPloc;
    int Sck, scon;
    char buffer[MAX_BUFFER];
    int bytesEnviats, bytesRebuts;
    int continuar = 1;

    // Creem el socket servidor. Posant el port 0, el SO n'assignarà un de lliure.
    Sck = TCP_CreaSockServidor("0.0.0.0", 0);
    if (Sck < 0) {
        perror("Error en crear el socket servidor");
        return -1;
    }

    // Com que hem usat el port 0, mirem quin port ens han assignat per saber on connectar el client
    if (TCP_TrobaAdrSockLoc(Sck, IPloc, &portTCPloc) == 0) {
        char msg[64];
        int len = snprintf(msg, sizeof(msg), "Servidor escoltant al port %d\n", portTCPloc);
        write(1, msg, len);
    }

    while (continuar) {
        scon = TCP_AcceptaConnexio(Sck, IPrem, &portTCPrem);
        if (scon < 0) {
            perror("Error en connectar amb el client"); 
        }
        else {   
            if (TCP_TrobaAdrSockLoc(scon, IPloc, &portTCPloc) == 0) {
                                    
                char portLoc[16];
                char portRem[16];
                
                sprintf(portLoc, "%d", portTCPloc);
                sprintf(portRem, "%d", portTCPrem);
                
                write(1, "@s1 (Local): ", 13);
                write(1, IPloc, strlen(IPloc));
                write(1, ":", 1); 
                write(1, portLoc, strlen(portLoc));
                write(1, "\n", 1);

                write(1, "@s2 (Remot): ", 13);
                write(1, IPrem, strlen(IPrem));
                write(1, ":", 1);
                write(1, portRem, strlen(portRem));
                write(1, "\n", 1);
            }

            int rep = 1;
            while (rep) {
                bytesRebuts = TCP_Rep(scon, buffer, MAX_BUFFER - 1);
                if (bytesRebuts < 0) {
                    perror("Error rebre dades");
                    rep = 0;
                } 
                else if (bytesRebuts == 0) {
                    write(1, "Connexió tancada pel client.\n", 29);
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
                    }
                }
            }
            TCP_TancaSock(scon);
        }   
    }
    TCP_TancaSock(Sck);
    return 0;
}