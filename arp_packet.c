#include <stdlib.h>     // malloc()
#include <arpa/inet.h>  // htons()
#include <stdio.h>      // perror()
#include <errno.h>      // errno
#include <linux/if_ether.h> // ETH_P_ARP

#include "arp_packet.h"
#include "String.h"

#define MAC_SIZE sizeof(MacAddr)
#define IP_SIZE  sizeof(IpAddr)

#define PLEN  0x04
#define ARP_OPER_SEND htons(0x0001)
#define ARP_OPER_RECV htons(0x0002)

typedef struct in_addr in_addr;

void set_ether_header(EtherHeader *etherHeader) {
    // set broadcast target MAC
    memset(etherHeader->trgMac, 0xFF, MAC_SIZE);
    // set ARP ethernet type
    etherHeader->etherType = htons(ETH_P_ARP);
}

void set_arp_header(ArpHeader *arpHeader){
    arpHeader->hwType = htons(ETH_P_802_3);
    arpHeader->plType = htons(ETH_P_IP);
    arpHeader->hwLen  = htons(ETH_P_802_3_MIN);
    arpHeader->plLen  = PLEN;
    arpHeader->op     = ARP_OPER_SEND;

    memset(arpHeader->trgMac, 0, MAC_SIZE);
    
    // Set submask
    memset(arpHeader->padding, 0, sizeof(arpHeader->padding));
}

void set_pack_addr(ArpPack *arpPack, Interface *interface){
    memcpy(arpPack->etherHeader.srcMac, interface->mac, MAC_SIZE);
    memcpy(arpPack->arpHeader.srcMac, arpPack->etherHeader.srcMac, MAC_SIZE);
    memcpy(arpPack->arpHeader.srcIp, interface->ipv4, IP_SIZE);
}

void set_arp_target(ArpHeader *arpHeader, uint8_t *ip) {
    memcpy(arpHeader->trgIp, ip, IP_SIZE);
}

void set_arp_pack(ArpPack *arpPack, Interface *interface){
    set_ether_header(&arpPack->etherHeader);
    set_arp_header(&arpPack->arpHeader);
    set_pack_addr(arpPack, interface);
}

