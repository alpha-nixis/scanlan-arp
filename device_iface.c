#include <errno.h>          // errno
#include <unistd.h>         // close()
#include <sys/socket.h>     // socket()
#include <sys/ioctl.h>      // ioctl()
#include <net/if.h>         // if_nametoindex() SIOCGIFHWADDR ...
#include <arpa/inet.h>      // htons()
#include <stdio.h>          // printf()

#include "String.h"
#include "device_iface.h"

#define CHECK_BITS(data, mask)    ~( (data) | (~(mask)) )
#define FILTERED    0

static void _log_ip(uint8_t *, uint8_t *);

void set_interface(Interface *interface, uint8_t *name){

    if (name == NULL){
        // get nearest interface with is UP
        struct if_nameindex *if_ni, *i;
        
        // get all interfaces
        if_ni = if_nameindex();
        if (if_ni == NULL){
            perror("get interfaces failed");
            return;
        }

        for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++){
            // checking whether interface is up
            set_ifr_device( (uint8_t *) i->if_name, SIOCGIFFLAGS );
            if (errno != 0){
                if_freenameindex(if_ni);
                fprintf(stderr, "ERROR: get interface %s flag: %s\n", i->if_name, strerror(errno));
                return;
            }
            
            if (CHECK_BITS(ifreq.ifr_flags, IFF_UP | IFF_RUNNING) == FILTERED && CHECK_BITS(ifreq.ifr_flags, IFF_LOOPBACK) != FILTERED){
                strncpy((char *) interface->name, ifreq.ifr_name, sizeof(interface->name) - 1);
                break;
            }

        }

        // free resourse for interfaces vector
        if_freenameindex(if_ni);
        if (interface->name[0] == 0){
            fprintf(stderr, "ERROR: There is no UP interfaces\n");
            errno = 1;
            return;
        }
        
    } else {
        
        // check interface for UP and RUNNING flags and write interface name from argv[] argument
        set_ifr_device(name, SIOCGIFFLAGS);
        if (errno != 0){
            fprintf(stderr, "ERROR: get interface %s flag: %s\n", name, strerror(errno));
            return;
        }

        if (CHECK_BITS(ifreq.ifr_flags, IFF_UP | IFF_RUNNING) != FILTERED){
            fprintf(stderr, "ERROR: interface %s is not UP or (and) RUNNING\n", name);
            errno = 1;
            return;
        }

        strncpy((char *) interface->name, (char *) name, sizeof(interface->name));
    }
    
    set_interface_addrs(interface);
    set_device(interface);

}

void set_interface_addrs(Interface *interface){
    set_ip(interface);
    set_mac(interface);
    set_netmask(interface);
    set_broadcast(interface);
    set_network(interface);
}

void set_mac(Interface *interface){
    set_ifr_device(interface->name, SIOCGIFHWADDR);
    // Copying MAC to buffer
    memcpy(interface->mac, ifreq.ifr_hwaddr.sa_data, sizeof(interface->mac));
}

void set_ip(Interface *interface){
    set_ifr_device(interface->name, SIOCGIFADDR);
    // Copying IP to buffer
    sockAddr = (struct sockaddr_in *) &ifreq.ifr_addr;
    memcpy(interface->ipv4, &sockAddr->sin_addr, sizeof(interface->ipv4));
}

void set_netmask(Interface *interface) {
    set_ifr_device(interface->name, SIOCGIFNETMASK);
    // Copy netmask in buffer
    sockAddr = (struct sockaddr_in *) &ifreq.ifr_netmask;
    memcpy(interface->netmask, &sockAddr->sin_addr, sizeof(interface->netmask));
}

void set_broadcast(Interface *interface){
    set_ifr_device(interface->name, SIOCGIFBRDADDR);
    // Copy broadcast in buffer
    sockAddr = (struct sockaddr_in *) &ifreq.ifr_broadaddr;
    memcpy(interface->broadcast, &sockAddr->sin_addr, sizeof(interface->broadcast));
}

void set_network(Interface *interface) {
    uint32_t network = *(uint32_t *) interface->netmask & *(uint32_t *) interface->ipv4;
    memcpy(interface->network, &network, sizeof(network));
}

void set_device(Interface *interface) {
    // setting device structure
    if ((interface->device.sll_ifindex = if_nametoindex((char *) interface->name)) == 0){
        fprintf(stderr, "ERROR: Cannot get index interface: %s\n", strerror(errno));
        return;
    }

    interface->device.sll_family = AF_PACKET;
    memcpy(interface->device.sll_addr, interface->mac, sizeof(interface->mac));
    interface->device.sll_halen = htons(6);
}

void set_ifr_device(uint8_t *name, unsigned long flags){
    int sock_desc;

    memset(&ifreq, 0, sizeof(Ifreq));
    snprintf(ifreq.ifr_name, sizeof(ifreq.ifr_name), "%s", name);

    if ((sock_desc = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0){
        fprintf(stderr, "ERROR: Open socket for ioctl\n");
        return;
    }
    
    if (ioctl(sock_desc, flags, &ifreq) < 0){
        perror("ioctl() request");
    }

    close(sock_desc);
}

void log_interface(Interface *interface){
    printf("Interface:\n\t");
    printf("name: \t\t%s\n", interface->name);
    _log_ip(interface->ipv4,        (uint8_t *) "\tipv4:\t\t");
    _log_ip(interface->netmask,     (uint8_t *) "\tnetmask:\t");
    _log_ip(interface->broadcast,   (uint8_t *) "\tbroadcast:\t");
    
    printf("\tmac:\t\t");
    for (uint8_t i = 0; i < 5; i++){
        printf("%02x:", interface->mac[i]);
    }
    printf("%02x\n", interface->mac[5]);

    _log_ip(interface->network,     (uint8_t *) "\tnetwork:\t");
}

static void _log_ip(uint8_t *ip, uint8_t *str){
    printf("%s", str);
    for (uint8_t i = 0; i < 3; i++){
        printf("%d.", ip[i]);
    }
    printf("%d\n", ip[3]);
}
