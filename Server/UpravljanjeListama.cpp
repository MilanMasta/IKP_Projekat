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
        temp = temp->next;
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