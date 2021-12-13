#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <list>
#include <iostream> 
#include <thread>   
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP server that use blocking sockets

typedef struct poruka_st {
    char sadrzajPoruke[BUFFER_SIZE];
    char posiljaoca[20];
    struct poruka_st* next;
} PORUKA;

typedef struct klijent_st {
    char ime[20];
    struct klijent_st* next;
    PORUKA* poruke;
} KLIJENT;

void init_list(KLIJENT** head){
    *head = NULL;
}

void add_to_list(KLIJENT* novi, KLIJENT** head) {
    if (*head == NULL) { // list is empty
        *head = novi;
        return;
    }

    add_to_list(novi, &((*head)->next));
}

KLIJENT* kreiraj_novog_klijenta(char ime[]) {
    KLIJENT* novi = (KLIJENT*)malloc(sizeof(KLIJENT));
    if (novi == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }

    strcpy(novi->ime, ime);
    PORUKA* poruke = NULL;
    novi->next = NULL;
    printf("%s kreiran\n", ime);
    return novi;
}

void dodaj_poruku_u_inbox(char ime[],char NoviSadrzajPoruka[],char posiljaoc[]) {
    PORUKA* novaPoruka = (PORUKA*)malloc(sizeof(PORUKA));
    if (novaPoruka == NULL) {
        printf("Not enough RAM!\n");
        exit(21);
    }

    strcpy(novaPoruka->sadrzajPoruke, NoviSadrzajPoruka);
}

int brojKorisnika = 0;

int main()
{
    KLIJENT* head;
    init_list(&head);

    
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
        return 1;
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
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address to socket
    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Check if socket is successfully binded to address and port from sockaddr_in structure
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    
    printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

    do
    {
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
            return 1;
        }

        printf("\nNew client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        do
        {
            // Receive data until the client shuts down the connection
            iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);

            if (iResult > 0)	// Check if message is successfully received
            {
                dataBuffer[iResult] = '\0';
                   
                if (brojKorisnika == 0) {
                    KLIJENT* novi =  kreiraj_novog_klijenta(dataBuffer);
                    add_to_list(novi, &head);
                        brojKorisnika++;
                }
                else {
                    //provjera da li INBOX vec postoji treba da se uradi
                    KLIJENT* novi = kreiraj_novog_klijenta(dataBuffer);
                    add_to_list(novi, &head);
                    brojKorisnika++;
                }
                printf("Client sent: %s.\n", dataBuffer);

            }
            else if (iResult == 0)	// Check if shutdown command is received
            {
                // Connection was closed successfully
                printf("Connection with client closed.\n");
                closesocket(acceptedSocket);
            }
            else	// There was an error during recv
            {

                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(acceptedSocket);
            }

        } while (iResult > 0);

        // Here is where server shutdown loguc could be placed

    } while (true);

    // Shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

    //Close listen and accepted sockets
    closesocket(listenSocket);
    closesocket(acceptedSocket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}