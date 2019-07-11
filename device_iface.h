#pragma once
#ifndef DEVICE_IFACE_H
#define DEVICE_IFACE_H

#include <stdint.h>
#include <net/if.h>          // IFNAMIZ
#include <linux/if_packet.h> // struct sockaddr_ll

typedef struct {
    struct sockaddr_ll device;
    uint8_t name[IFNAMSIZ];
    uint8_t ipv4[4];
    uint8_t netmask[4];
    uint8_t broadcast[4];
    uint8_t mac[6];
    uint8_t network[4];
} Interface;

typedef struct ifreq Ifreq;

struct sockaddr_in *sockAddr;

// global ifreq variable
Ifreq ifreq;

void set_interface(Interface *, uint8_t *);

void set_interface_addrs(Interface *);

void set_mac(Interface *);
void set_ip(Interface *);
void set_netmask(Interface *);
void set_broadcast(Interface *);
void set_network(Interface *);

void set_device(Interface *);

void set_ifr_device(uint8_t *, unsigned long);

void log_interface(Interface *);
#endif
