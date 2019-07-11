#ifndef ARP_PACKET_H
#define ARP_PACKET_H

#include <stdint.h>

#include "device_iface.h"

#define MAC_SIZE    sizeof(MacAddr)
#define IP_SIZE     sizeof(IpAddr)

typedef uint8_t MacAddr[6];
typedef uint8_t IpAddr[4];

typedef struct {
    MacAddr trgMac;
    MacAddr srcMac;
    uint16_t etherType;
} EtherHeader;

typedef struct {
    uint16_t hwType;
    uint16_t plType;
    uint8_t hwLen;
    uint8_t plLen;
    uint16_t op;
    
    MacAddr srcMac;
    IpAddr  srcIp;

    MacAddr trgMac;
    IpAddr  trgIp;

    uint8_t padding[18];
} ArpHeader;

typedef struct {
    EtherHeader etherHeader;
    ArpHeader   arpHeader;
} ArpPack;

// Creating ARP frame
void set_arp_pack(ArpPack *, Interface *);
void set_ether_header(EtherHeader *);
void set_arp_header(ArpHeader *);
void set_pack_addr(ArpPack *, Interface *);
void set_arp_target(ArpHeader *, uint8_t *);

#endif
