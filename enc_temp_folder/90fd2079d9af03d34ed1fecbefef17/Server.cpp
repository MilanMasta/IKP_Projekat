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
KLIJENT* head = NULL;



DWORD WINAPI inboxKorisnika(LPVOID lpParam) {

 KLIJENT* klijent = (KLIJENT*)lpParam;
 head;
 unsigned int klijentSocket = klijent->acptSocket;
 char* ime = klijent->ime;
 char dataBuffer[BUFFER_SIZE];
 int iResult = 1;
 head;
 while (true) {
     while (iResult > 0)
     {
         // Receive data until the client shuts down the connection
         iResult = recv(klijentSocket, dataBuffer, BUFFER_SIZE, 0);
         if (iResult > 0)	// Check if message is successfully received
         {
             dataBuffer[iResult] = '\0';
             char* primalac = strchr(dataBuffer, '\n') + 1;
             char* poruka = strtok(dataBuffer, "\n");
             // Log message text
             printf("%s Je poslao poruku klijentu %s: %s\n", klijent->ime, primalac, poruka);

         }
         else if (iResult == 0)	// Check if shutdown command is received
         {
             // Connection was closed successfully
             printf("Connection with client closed.\n");
             closesocket(klijentSocket);
         }
         else	// There was an error during recv
         {
             printf("Klijent %s se diskonektovao\n", klijent->ime);
             closesocket(klijentSocket);
             iResult = -1;
             iResult = shutdown(klijentSocket, SD_BOTH);
             closesocket(klijentSocket);
         }
     } ;
 }

 return 0;
}

DWORD WINAPI KonekcijaKorisnika(LPVOID lpParam)
{
    KLIJENT** head = (KLIJENT**)lpParam;
    //inicijalizacija liste
    int brojKorisnika = 0;
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;

    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer used for storing incoming data
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
    }


    // Initialize serverAddress structure used by bind
    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;				// IPv4 address family
    serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
    serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address family
        SOCK_STREAM,  // Stream socket
        IPPROTO_TCP); // TCP protocol

// Check if socket is successfully created
    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
    }

    // Setup the TCP listening socket - bind port number and local address to socket
    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Check if socket is successfully binded to address and port from sockaddr_in structure
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

    while (true) {
        // Struct for information about connected client
        sockaddr_in clientAddr;

        int clientAddrSize = sizeof(struct sockaddr_in);

        // Accept new connections from clients 
        acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

        // Check if accepted socket is valid 
        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
        }

        char ime[20];
        // Receive data until the client shuts down the connection
        iResult = recv(acceptedSocket, ime, 20, 0);

        if (iResult > 0)	// Check if message is successfully received
        {
            ime[iResult] = '\0';

            if (brojKorisnika == 0) {
                KLIJENT* klijent = kreiraj_novog_klijenta(ime,acceptedSocket);
                add_to_list(klijent, head);
                brojKorisnika++;
                DWORD KorisnikID;
                HANDLE hinboxKorisnika;
                hinboxKorisnika = CreateThread(NULL, 0, &inboxKorisnika, (LPVOID)klijent, 0, &KorisnikID);
            }
            else {
                KLIJENT* ret = pronadjiKlijenta(head, ime);
                if (ret == NULL) { // ne postoji taj klijent
                    KLIJENT* klijent = kreiraj_novog_klijenta(ime, acceptedSocket);
                    add_to_list(klijent, head);
                    brojKorisnika++;
                    DWORD KorisnikID;
                    HANDLE hinboxKorisnika;
                    hinboxKorisnika = CreateThread(NULL, 0, &inboxKorisnika, (LPVOID)klijent, 0, &KorisnikID);
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
                            hinboxKorisnika = CreateThread(NULL, 0, &inboxKorisnika, (LPVOID)klijent, 0, &KorisnikID);
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


#pragma region UpravljanjeListom



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
    head;
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
    printf("%s se konektovao\n", ime);
    return novi;
}

void dodaj_poruku_u_inbox(char* NoviSadrzajPoruka,char* posiljaoc,char* primalac) {
    PORUKA* novaPoruka = (PORUKA*)malloc(sizeof(PORUKA));
    if (novaPoruka == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }
    strcpy(novaPoruka->sadrzajPoruke, NoviSadrzajPoruka);
    strcpy(novaPoruka->posiljaoc, posiljaoc);
}
#pragma endregion

int main()
{
    DWORD KonekcijaKorisnikaID;
    HANDLE hKonekcijaKorisnika;
    hKonekcijaKorisnika = CreateThread(NULL, 0, &KonekcijaKorisnika, (LPVOID)&head, 0, &KonekcijaKorisnikaID);
    

    _getch();
    return 0;
}

