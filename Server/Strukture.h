typedef struct poruka_st {
    char sadrzajPoruke[256];
    char posiljaoc[20];
    struct poruka_st* next;
} PORUKA;

typedef struct klijent_st {
    char ime[20];
    unsigned int acptSocket;
    PORUKA* poruke;
    struct klijent_st* next;
} KLIJENT;
