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
using namespace std;
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP server that use blocking sockets
#pragma region Strukture
typedef struct poruka_st {
    char sadrzajPoruke[BUFFER_SIZE];
    char posiljaoc[20];
    struct poruka_st* next;
} PORUKA;

typedef struct klijent_st {
    char ime[20];
    unsigned int acptSocket;
    PORUKA* poruke;
    struct klijent_st* next;
} KLIJENT;

#pragma endregion

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



             if (!strcmp(primalac, "")) { // TRAZI PORUKE DA VIDI
                 int a = 1;
                 while (a == 1) { // nece da ih gleda vise
                     KLIJENT** inboxKlijenta = pronadjiKlijentaZaPrijemPoruke(&head, ime);
                     PORUKA** poruka = &(*inboxKlijenta)->poruke;
                     if (*poruka != NULL) {
                         char* pos = (*poruka)->posiljaoc;
                         char* sadrzaj = (*poruka)->sadrzajPoruke;
                         strcat(strcat(strcat(pos, " : "), sadrzaj), "\n");
                         iResult = send(klijentSocket, pos, (int)strlen(pos), 0);
                         (*poruka) = (*poruka)->next;
                     }
                     unsigned long mode = 1; //non-blocking mode
                     iResult = ioctlsocket(klijentSocket, FIONBIO, &mode);
                     iResult = recv(klijentSocket, dataBuffer1, BUFFER_SIZE, 0);
                     if (iResult != SOCKET_ERROR) {
                         dataBuffer1[iResult] = '\0';
                         if (!strcmp(dataBuffer1, "kraj")) {
                             mode = 0; //blocking mode
                             iResult = ioctlsocket(klijentSocket, FIONBIO, &mode);
                             a = 0;
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
                     KLIJENT* klijent2 = kreiraj_novog_klijenta(primalac, -1);
                     klijent2->poruke = poruka1;
                     add_to_list(klijent2, &head);
                     brojKorisnika++;
                     printf("%s je poslao poruku klijentu %s: %s\n", klijent->ime, primalac, poruka);
                     citajKlijenta(&head);
                 }
             }
         }
         else if (iResult == 0)
         {
             printf("Klijent %s se diskonektovao\n", klijent->ime);
             closesocket(klijentSocket);
             iResult = -1;
             iResult = shutdown(klijentSocket, SD_BOTH);
             closesocket(klijentSocket);
             return 1;
         }
         else
         {
             printf("Klijent %s se diskonektovao\n", klijent->ime);
             closesocket(klijentSocket);
             iResult = -1;
             iResult = shutdown(klijentSocket, SD_BOTH);
             closesocket(klijentSocket);
             return 1;
         }
 }

 return 0;
}

DWORD WINAPI KonekcijaKorisnika(LPVOID lpParam)
{
#pragma region Inicijalizacija

    KLIJENT** head = (KLIJENT**)lpParam;
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
                KLIJENT* klijent = kreiraj_novog_klijenta(ime,acceptedSocket);
                printf("%s se konektovao\n", ime);
                add_to_list(klijent, head);
                brojKorisnika++;
                DWORD KorisnikID;
                HANDLE hinboxKorisnika;
                hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
            }
            else {
                KLIJENT* ret = pronadjiKlijenta(head, ime);
                if (ret == NULL) { // ne postoji taj klijent
                    KLIJENT* klijent = kreiraj_novog_klijenta(ime, acceptedSocket);
                    printf("%s se konektovao\n", ime);
                    add_to_list(klijent, head);
                    brojKorisnika++;
                    DWORD KorisnikID;
                    HANDLE hinboxKorisnika;
                    hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
                    citajKlijenta(head);
                }
                else {
                    KLIJENT* klijent = (*head);
                    while (klijent != NULL)
                    {
                        if (!strcmp(klijent->ime, ime)) {
                            klijent->acptSocket = acceptedSocket;
                            DWORD KorisnikID;
                            HANDLE hinboxKorisnika;
                            hinboxKorisnika = CreateThread(NULL, 0, &PrikupljanjePoruka, (LPVOID)klijent, 0, &KorisnikID);
                            printf("%s se opet konektovao\n", ime);
                        }
                        klijent = klijent->next;
                    }
                    citajKlijenta(head);
                }
            }
        }
    }
    return 0;
}



#pragma region UpravljanjeListama
void dodajPorukuUListu(PORUKA* novi, PORUKA** head) {
    if (*head == NULL) { // list is empty
        *head = novi;
        return;
    }
    dodajPorukuUListu(novi, &((*head)->next));
}

PORUKA* kreirajPoruku(char* NoviSadrzajPoruka, char* posiljaoc) {
    PORUKA* novaPoruka = (PORUKA*)malloc(sizeof(PORUKA));
    if (novaPoruka == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }

    strcpy(novaPoruka->sadrzajPoruke, NoviSadrzajPoruka);
    strcpy(novaPoruka->posiljaoc, posiljaoc);
    novaPoruka->next = NULL;

    return novaPoruka;
}

KLIJENT** pronadjiKlijentaZaPrijemPoruke(KLIJENT** head, char* imeKlijenta) {
    KLIJENT* ret = NULL;
    KLIJENT* temp = *head;
    while (temp != NULL)
    {
        if (!strcmp(temp->ime, imeKlijenta)) {
            return &temp;
        }
        temp = temp->next;
    }
    return NULL;
}
KLIJENT* pronadjiKlijenta(KLIJENT** head, char* imeKlijenta) {
    KLIJENT* ret = NULL;
    KLIJENT* temp = *head;
    while (temp != NULL)
    {
        if (!strcmp(temp->ime, imeKlijenta)) {
            return temp;
        }
        temp = temp ->next;
    }
    return ret;
}

void citajKlijenta(KLIJENT** head) {
    KLIJENT* temp = *head;
    while (temp != NULL)
    {
        printf("%s %u \n", temp->ime, temp->acptSocket);
        temp = temp->next;
    }
}

void add_to_list(KLIJENT* novi, KLIJENT** head) {
    if (*head == NULL) { // list is empty
        *head = novi;
        return;
    }
    add_to_list(novi, &((*head)->next));
}

KLIJENT* kreiraj_novog_klijenta(char* ime, unsigned int socket) {
    KLIJENT* novi = (KLIJENT*)malloc(sizeof(KLIJENT));

    if (novi == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }

    strcpy(novi->ime, ime);
    novi->acptSocket = socket;
    novi->next = NULL;
    novi->poruke = NULL;
    return novi;
}
#pragma endregion

int main()
{
    DWORD KonekcijaKorisnikaID;
    HANDLE hKonekcijaKorisnika;
    hKonekcijaKorisnika = CreateThread(NULL, 0, &KonekcijaKorisnika, (LPVOID)&head, 0, &KonekcijaKorisnikaID);
    
    while (_kbhit) {

    }
    return 0;
}

