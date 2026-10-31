#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "p1-tTCP.h"

#define MAX_BUFFER 200
#define LINIA_FINAL "Final"

int main(int argc, char *argv[])
{
    char IPrem[16];
    int portTCPrem;
    char IPloc[16];
    int portTCPloc;
    int Sck;
    char buffer[MAX_BUFFER];
    int bytesEnviats, bytesRebuts, n;
    char resposta[10];
    int continuar = 1;

    while (continuar) {
        Sck = TCP_CreaSockClient("0.0.0.0", 0);
        if (Sck < 0) {
            perror("Error en crear el socket client");
            return -1;
        }

        write(1, "Introdueix l'@IP del servidor remot: ", 37);
        n = read(0, IPrem, sizeof(IPrem) - 1);
        if (n > 0) {
            if (IPrem[n - 1] == '\n') IPrem[n - 1] = '\0'; // Neteja el salt de línia
            else IPrem[n] = '\0';
        }

        write(1, "Introdueix el port TCP del servidor: ", 37);
        n = read(0, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            if (buffer[n - 1] == '\n') buffer[n - 1] = '\0';
            else buffer[n] = '\0';
            portTCPrem = atoi(buffer);
        }

        if (TCP_DemanaConnexio(Sck, IPrem, portTCPrem) < 0) {
            perror("Error en connectar amb el servidor");
            TCP_TancaSock(Sck);
        }
        else {
            if (TCP_TrobaAdrSockLoc(Sck, IPloc, &portTCPloc) == 0 &&
                TCP_TrobaAdrSockRem(Sck, IPrem, &portTCPrem) == 0) {

                char portLoc[16];
                char portRem[16];
                
                sprintf(portLoc, "%d", portTCPloc);
                sprintf(portRem, "%d", portTCPrem);
                
                write(1, "@s1: ", 5);
                write(1, IPloc, strlen(IPloc));
                write(1, ":", 1);
                write(1, portLoc, strlen(portLoc));
                write(1, "\n", 1);

                write(1, "@s2: ", 5);
                write(1, IPrem, strlen(IPrem));
                write(1, ":", 1);
                write(1, portRem, strlen(portRem));
                write(1, "\n", 1);
            }

            int envia = 1;
            while (envia) {
                write(1, "Escriu un missatge ('Final' per sortir): ", 41);
                n = read(0, buffer, MAX_BUFFER - 1);
                if (n <= 0) {
                    envia = 0;
                }
                else {
                    if (buffer[n - 1] == '\n') {
                        buffer[n - 1] = '\0';
                        n--; //no contem el byte del salt de linea
                    } else {
                        buffer[n] = '\0';
                    }

                    if (strcmp(buffer, LINIA_FINAL) == 0) {
                        envia = 0;
                    }
                    else {
                        bytesEnviats = TCP_Envia(Sck, buffer, n); // Enviem només els caràcters útils (n)
                        if (bytesEnviats < 0) {
                            perror("Error enviar dades");
                            envia = 0;
                        }
                        else {
                            char msg[64];
                            int num = snprintf(msg, sizeof(msg), "Bytes enviats: %d\n", bytesEnviats);
                            write(1, msg, num);

                            bytesRebuts = TCP_Rep(Sck, buffer, MAX_BUFFER - 1);
                            if (bytesRebuts < 0) {
                                perror("Error rebre dades");
                                envia = 0;
                            }
                            else if (bytesRebuts == 0) {
                                write(1, "Connexió tancada pel servidor.\n", 31);
                                envia = 0;
                            }
                            else {
                                buffer[bytesRebuts] = '\0'; // Assegurem el final de l'string abans d'imprimir
                                write(1, "Rebut: ", 7);
                                write(1, buffer, strlen(buffer));
                                write(1, "\n", 1);
                            }
                        }
                    }
                }
            }
            TCP_TancaSock(Sck);
        }
        
        write(1, "\nVols connectar-te a un altre servidor? (s/n): ", 47);
        n = read(0, resposta, sizeof(resposta) - 1);
        if (n > 0) {
            if (resposta[n - 1] == '\n') resposta[n - 1] = '\0';
            else resposta[n] = '\0';
            
            if (strcmp(resposta, "s") != 0 && strcmp(resposta, "S") != 0) {
                continuar = 0;
            }
        }
    }
    return 0;
}
