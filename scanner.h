#ifndef SCANNER_H
#define SCANNER_H

#include <unistd.h> // useconds_t

#include "arp_packet.h"
#include "device_iface.h"

typedef struct {
    uint8_t *interface_name;
    FILE *file; // output file
    useconds_t delay; // delays between sending ARP packets
} Conf;

typedef struct {
    int sock_desc;
    void **addrList; // addrList wich represend ip and mac adresses via trie data structure
    Conf *config;
    Interface interface;
    ArpPack arpPack;
} Scanner;

Scanner *create_scanner(Conf *);

void start_scan(Scanner *);
void clean_scanner(Scanner *);
void send_arp_pack( Scanner *);
void get_addrs(Scanner *);
void out_addr(Scanner *, uint8_t *, uint8_t *);

void _add_addr(Scanner *, uint8_t *, uint8_t *); // adding ip and mac to Scanner addrList
void *_start_listen(void *);
#endif
