#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP client that use blocking sockets
int main()
{
    // Socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    char dataBuffer[BUFFER_SIZE];
    char ime[20];
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;								// IPv4 protocol
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
    serverAddress.sin_port = htons(SERVER_PORT);					// server port
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Unesi ime: ");
    gets_s(ime, sizeof(char) * 20);
    iResult = send(connectSocket, ime, (int)strlen(ime), 0);

    int petlja = 1;
    
    while (petlja != 0) {
        int a = 0;
        char primalac[20] = "";
        printf("1. Inbox.\n2. Posalji poruku.\n");
        char izbor1[sizeof(int)];
        gets_s(izbor1, sizeof(int)); 
        int izbor = atoi(izbor1);
        char zahtjev[] = "Zelim poruke\n";
        switch (izbor) {
        case 1:
            printf("\t\t\t\t\t\tOVO JE VAS INBOX: \n");
            iResult = send(connectSocket, zahtjev, (int)strlen(zahtjev), 0); // zahtjev za pregled porukas
            while (!_kbhit()) {
                

                unsigned long mode = 1; //non-blocking mode
                iResult = ioctlsocket(connectSocket, FIONBIO, &mode);
                if (iResult != NO_ERROR)
                    printf("ioctlsocket failed with error: %ld\n", iResult);


                iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
                //Provera da li se operacije uspesno izvrsila
                if (iResult != SOCKET_ERROR) {
                    dataBuffer[iResult] = '\0';
                    printf(dataBuffer);
                    Sleep(1500);
                }
                else
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) {
                        // U pitanju je blokirajuca operacija koja zbog rezima
                        // soketa nece biti izvrsena
                    }
                    else {
                        // Desila se neka druga greska prilikom poziva operacije
                    }
                }
            }
            iResult = send(connectSocket, "kraj", 4, 0);
            break;

        case 2:
            while (a != 1) {
                strcpy(primalac, "");
                while (!strcmp(primalac, "")) {
                    printf("Primalac (1 za povratak u meni):");
                    gets_s(primalac, sizeof(primalac));
                }
                if (strcmp(primalac,"1")) {
                printf("Posalji poruku:  ");
                gets_s(dataBuffer, BUFFER_SIZE);
                strcat(dataBuffer, "\n");
                strcat(dataBuffer, primalac);
                a = atoi(dataBuffer);
                
                    // Send message to server using connected socket
                    iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer) + 24, 0);

                    // Check result of send function
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("send failed with error: %d\n", WSAGetLastError());
                        closesocket(connectSocket);
                        WSACleanup();
                    }
                    printf("Poruka uspjesno poslata\n");
                }
                else {
                    a = 1;
                }
            }
            break;
        case 0:
            petlja = 0;
        default:
            break;
        }
    }

    // For demonstration purpose
    printf("\nPress any key to exit: ");
    _getch();


    // Close connected socket
    closesocket(connectSocket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}