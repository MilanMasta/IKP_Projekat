#pragma once
#ifndef HEADER_FILE
#define HEADER_FILE
#include "Strukture.h"

void dodajPorukuUListu(PORUKA** novi, PORUKA** head);

PORUKA* kreirajPoruku(char* NoviSadrzajPoruka, char* posiljaoc);

KLIJENT** pronadjiKlijentaZaPrijemPoruke(KLIJENT** head, char* imeKlijenta);

KLIJENT* pronadjiKlijenta(KLIJENT** head, char* imeKlijenta);

void citajKlijenta(KLIJENT** head);

void add_to_list(KLIJENT* novi, KLIJENT** head);

KLIJENT* kreiraj_novog_klijenta(char* ime, unsigned int socket);

#endif