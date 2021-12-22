#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream> 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "string.h"
#include "Strukture.h"

using namespace std;
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP server that use blocking sockets


KLIJENT* kreiraj_novog_klijenta(char* ime, unsigned int socket);
void add_to_list(KLIJENT* novi, KLIJENT** head);
void citajKlijenta(KLIJENT** head);
KLIJENT* pronadjiKlijenta(KLIJENT** head, char* imeKlijenta);
KLIJENT** pronadjiKlijentaZaPrijemPoruke(KLIJENT** head, char* imeKlijenta);
PORUKA* kreirajPoruku(char* NoviSadrzajPoruka, char* posiljaoc);
void dodajPorukuUListu(PORUKA* novi, PORUKA** head);
KLIJENT* head = NULL;
int brojKorisnika = 0;


DWORD WINAPI PrikupljanjePoruka(LPVOID lpParam) {

 KLIJENT* klijent = (KLIJENT*)lpParam;
 unsigned int klijentSocket = klijent->acptSocket;
 char* ime = klijent->ime;
 char dataBuffer[BUFFER_SIZE];
 char dataBuffer1[BUFFER_SIZE];
 int iResult = 1;
 while (true) {

         iResult = recv(klijentSocket, dataBuffer, BUFFER_SIZE, 0);
         if (iResult > 0)	// Check if message is successfully received
         {
             dataBuffer[iResult] = '\0';
             char* primalac = strchr(dataBuffer, '\n') + 1;
             char* poruka = strtok(dataBuffer, "\n");


             if (!strcmp(primalac,"")) { // TRAZI PORUKE DA VIDI i nalazi se u inboxu
                 int a = 1;
                 
                     KLIJENT** inboxKlijenta = pronadjiKlijentaZaPrijemPoruke(&head, ime);
                     PORUKA** poruka = &(*inboxKlijenta)->poruke;

                     while (*poruka != NULL) { //posalji sve poruke koje su se nekupile u red do tog momenta
                         char* pos = (*poruka)->posiljaoc;
                         char* sadrzaj = (*poruka)->sadrzajPoruke;
                         strcat(strcat(strcat(pos, " : "), sadrzaj), "\n");
                         iResult = send(klijentSocket, pos, (int)strlen(pos), 0);
                         (*poruka) = (*poruka)->next;
                     }

                 while (a == 1)
                 { // ako je a != 1 ne zeli poruke vise da gleda
                     unsigned long mode = 1; //non-blocking mode
                     iResult = ioctlsocket(klijentSocket, FIONBIO, &mode);
                     iResult = recv(klijentSocket, dataBuffer1, BUFFER_SIZE, 0);
                     if (iResult != SOCKET_ERROR) {
                         dataBuffer1[iResult] = '\0';
                         if (!strcmp(dataBuffer1, "kraj")) {
                             unsigned long mode = 0; //non-blocking mode
                             iResult = ioctlsocket(klijentSocket, FIONBIO, &mode);
                             a = 0;
                         }
                     }
                     else
                     {
                         if (WSAGetLastError() == WSAEWOULDBLOCK) {

                             while (*poruka != NULL) { //ako ima novih poruka posalji ih
                                 char* pos = (*poruka)->posiljaoc;
                                 char* sadrzaj = (*poruka)->sadrzajPoruke;
                                 strcat(strcat(strcat(pos, " : "), sadrzaj), "\n");
                                 iResult = send(klijentSocket, pos, (int)strlen(pos), 0);
                                 (*poruka) = (*poruka)->next;
                             }
                         }
                         else {
                             printf("Klijent %s se diskonektovao\n", klijent->ime);
                             closesocket(klijentSocket);
                             iResult = -1;
                             iResult = shutdown(klijentSocket, SD_BOTH);
                             closesocket(klijentSocket);
                             return 0;
                         }

                     }
                 }
             }
             else {

                 KLIJENT** inboxPrimaoca = pronadjiKlijentaZaPrijemPoruke(&head, primalac);
                 if (inboxPrimaoca != NULL) { // Postoji klijent
                     PORUKA* poruka1 = kreirajPoruku(poruka, klijent->ime);
                     dodajPorukuUListu(poruka1, &((*inboxPrimaoca)->poruke));
                     printf("%s je poslao poruku klijentu %s: %s\n", klijent->ime, primalac, poruka);
                 }
                 else { // Ne Postoji klijent
                     PORUKA* poruka1 = kreirajPoruku(poruka, klijent->ime);
                     KLIJENT* klijent2 = kreiraj_novog_klijenta(primalac, 0);
                     klijent2->poruke = poruka1;
                     add_to_list(klijent2, &head);
                     brojKorisnika++;
                     printf("%s je poslao poruku klijentu %s: %s\n", klijent->ime, primalac, poruka);
                     citajKlijenta(&head);
                 }
             }
         }
         else
         {
             printf("Klijent %s se diskonektovao\n", klijent->ime);
             closesocket(klijentSocket);
             iResult = -1;
             iResult = shutdown(klijentSocket, SD_BOTH);
             closesocket(klijentSocket);
             return 0;
         }
 }

 return 0;
}


int main()
{
#pragma region Inicijalizacija

    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET acceptedSocket = INVALID_SOCKET;
    int iResult;
    char dataBuffer[BUFFER_SIZE];
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());

    }
    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
    }

    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    printf("Server socket is set to listening mode. Waiting for new connection requests.\n");
#pragma endregion
    while (true) {

        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(struct sockaddr_in);
        acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
        }

        char ime[20];
        iResult = recv(acceptedSocket, ime, 20, 0);

        if (iResult > 0)	// Check if message is successfully received
        {
            ime[iResult] = '\0';

            if (brojKorisnika == 0) {
                KLIJENT* klijent = kreiraj_novog_klijenta(ime, acceptedSocket);
                printf("%s se konektovao\n", ime);
                add_to_list(klijent, &head);
                brojKorisnika++;
                DWORD KorisnikID;
                HANDLE hinboxKorisnika;
                hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
            }
            else {
                KLIJENT* ret = pronadjiKlijenta(&head, ime);
                if (ret == NULL) { // ne postoji taj klijent
                    KLIJENT* klijent = kreiraj_novog_klijenta(ime, acceptedSocket);
                    printf("%s se konektovao\n", ime);
                    add_to_list(klijent, &head);
                    brojKorisnika++;
                    DWORD KorisnikID;
                    HANDLE hinboxKorisnika;
                    hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
                    citajKlijenta(&head);
                }
                else {
                    KLIJENT* klijent = (head);
                    while (klijent != NULL)
                    {
                        if (!strcmp(klijent->ime, ime)) {
                            unsigned int a = klijent->acptSocket;
                            klijent->acptSocket = acceptedSocket;
                            DWORD KorisnikID;
                            HANDLE hinboxKorisnika;
                            hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
                            if (a != 0) {
                                printf("%s se opet konektovao\n", ime);
                            }
                            else {
                                printf("%s se konektovao\n", ime);
                            }

                        }
                        klijent = klijent->next;
                    }
                    citajKlijenta(&head);
                }
            }
        }
    }
    
    return 0;
}

