/**************************************************************************/
/*                                                                        */
/* L'aplicació ECO amb sockets TCP/IP                                     */
/* Fitxer tTCP.c que "implementa" la capa de transport TCP, o més         */
/* ben dit, que encapsula les funcions de la interfície de sockets        */
/* TCP, en unes altres funcions més simples i entenedores: la "nova"      */
/* interfície de sockets TCP.                                             */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int TCP_FuncioExterna(arg1, arg2...) { }   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* són la "nova" interfície de la capa de transport TCP (la "nova"        */
/* interfície de sockets TCP).                                            */

/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket creat si tot va bé;                        */
/* -1 si hi ha error.                                                     */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc)
{
	int scon, i;
	struct sockaddr_in adrloc;

	if((scon=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		return -1;
	}

	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portTCPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc);
	for(i=0;i<8;i++){adrloc.sin_zero[i]='\0';}

	if((bind(scon,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1)
	{
		close(scon);
		return -1;
	}

	return scon;
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket creat si tot va bé;                        */
/* -1 si hi ha error.                                                     */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc)
{
	int sesc, i;
	struct sockaddr_in adrloc;

	if((sesc=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		return -1;
	}

	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portTCPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc);
	for(i=0;i<8;i++){adrloc.sin_zero[i]='\0';}

	if((bind(sesc,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1)
	{
		close(sesc);
		return -1;
	}

	if((listen(sesc,3))==-1)
	{
		close(sesc);
		return -1;
	}

	return sesc;
}

/* El socket TCP “client” d’identificador “Sck” es connecta al socket     */
/* TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot va bé    */
/* es diu que el socket “Sck” passa a l’estat “connectat” o establert     */
/* – established –). Recordeu que això vol dir que s’estableix una        */
/* connexió TCP (les dues entitats TCP s’intercanvien missatges           */
/* d’establiment de la connexió).                                         */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem)
{
	int i;
	struct sockaddr_in adrrem;

	adrrem.sin_family=AF_INET;
	adrrem.sin_port=htons(portTCPrem);
	adrrem.sin_addr.s_addr=inet_addr(IPrem);
	for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}

	if((connect(Sck,(struct sockaddr*)&adrrem,sizeof(adrrem)))==-1)
	{
		return -1;
	}

	return 0;
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/*                                                                        */
/* Omple “IPrem” i “portTCPrem” amb respectivament, l’@IP i el #port      */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket connectat creat si tot va bé;              */
/* -1 si hi ha error.                                                     */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem)
{
	int scon;
	socklen_t long_adrrem;
	struct sockaddr_in adrrem;

	long_adrrem=sizeof(adrrem);
	if((scon=accept(Sck,(struct sockaddr*)&adrrem,&long_adrrem))==-1)
	{
		return -1;
	}

	strcpy(IPrem,inet_ntoa(adrrem.sin_addr));
	*portTCPrem=ntohs(adrrem.sin_port);

	return scon;
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna:                                                               */
/*  el nombre de bytes enviats si tot va bé;                              */
/* -1 si hi ha error.                                                     */
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes)
{
	int bytes_escrits;

	if((bytes_escrits=write(Sck,SeqBytes,LongSeqBytes))==-1)
	{
		return -1;
	}

	return bytes_escrits;
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes” (que té una longitud de “LongSeqBytes” bytes), */
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna:                                                               */
/*  el nombre de bytes rebuts si tot va bé;                               */
/*  0 si la connexió està tancada;                                        */
/* -1 si hi ha error.                                                     */
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes)
{
	int bytes_llegits;

	if((bytes_llegits=read(Sck,SeqBytes,LongSeqBytes))==-1)
	{
		return -1;
	}

	return bytes_llegits;
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TancaSock(int Sck)
{
	if(close(Sck)==-1)
	{
		return -1;
	}

	return 0;
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc” i “portTCPloc” amb respectivament, la seva     */
/* @IP i #port TCP.                                                       */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc)
{
	socklen_t long_adrloc;
	struct sockaddr_in adrloc;

	long_adrloc=sizeof(adrloc);
	if((getsockname(Sck,(struct sockaddr*)&adrloc,&long_adrloc))==-1)
	{
		return -1;
	}

	strcpy(IPloc,inet_ntoa(adrloc.sin_addr));
	*portTCPloc=ntohs(adrloc.sin_port);

	return 0;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem” i             */
/* “portTCPrem” amb respectivament, la seva @IP i #port TCP.              */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem)
{
	socklen_t long_adrrem;
	struct sockaddr_in adrrem;

	long_adrrem=sizeof(adrrem);
	if((getpeername(Sck,(struct sockaddr*)&adrrem,&long_adrrem))==-1)
	{
		return -1;
	}

	strcpy(IPrem,inet_ntoa(adrrem.sin_addr));
	*portTCPrem=ntohs(adrrem.sin_port);

	return 0;
}

/* Obté un missatge de text de l'S.O. que descriu l'error produït en      */
/* la darrera crida de sockets TCP, i omple "CodiRes" amb la variable     */
/* errno de l'S.O., un codi identificador d'aquest missatge de text       */
/*                                                                        */
/* Retorna:                                                               */
/*  aquest missatge de text en un "string" de C (vector de chars          */
/*  imprimibles acabat en '\0').                                          */
char* T_ObteTextRes(int *CodiRes)
{
 *CodiRes= errno;
 return strerror(errno);
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int TCP_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */