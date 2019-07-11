#include <stdlib.h>         // malloc
#include <stdio.h>          // printf()
#include <unistd.h>         // close() sleep()
#include <sys/socket.h>     // socket()
#include <sys/ioctl.h>      // ioctl()
#include <arpa/inet.h>      // htons()
#include <netinet/ether.h>  // ETH_P_ALL ETH_P_ARP
#include <errno.h>          // perror()
#include <pthread.h>        // multithreading (pthread_create)

#include "String.h"
#include "scanner.h"
#include "device_iface.h"
#include "arp_packet.h"

#define ARP_PACK_SIZE   sizeof(ArpPack)

static void _recur_clean(void **, uint8_t); // cleaning Trie tree via recursion

typedef enum {UNDEFINED, START, STOP_LISTEN} ListenState;

ListenState listenState;

Scanner *create_scanner(Conf *config){
    Scanner *scan = malloc(sizeof(*scan));
    if (scan == NULL){
        perror("cannot allocate memery for scanner. In scanner.c, function createScanner");
        return NULL;
    }
    
    scan->config = config;

    // Setting interface
    set_interface(&scan->interface, config->interface_name);
    if (errno != 0){
        fprintf(stderr, "ERROR: Aborting interface setting\n");
        return scan;
    }
    
    
    // creating ARP package
    set_arp_pack(&scan->arpPack, &scan->interface);

    // open socket for sending ARP packets
    if ((scan->sock_desc = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0){
        perror("cannot open socket for sending ARP");
        return scan;
    }
    
    return scan;
}

void start_scan(Scanner *scan){
    uint8_t *networkIp = scan->interface.network;
    uint8_t *mask     = scan->interface.netmask;
    uint32_t i = 0;
    uint32_t h;
    uint32_t res;
    
    pthread_t listenThreadId;

    // Listen all incoming ARP packets Thread
    pthread_create(&listenThreadId, NULL, _start_listen, scan);
    // waiting while thread is start
    while (listenState != START) {};

    // Sending ARP packets for all posibile IP's
    do {
        // generate ARP packet
        h = htonl(i);
        res = h | *((uint32_t *) networkIp);
        memcpy(&scan->arpPack.arpHeader.trgIp, &res, IP_SIZE);
        
        // sending ARP packet
        send_arp_pack(scan);
        
        // Out data in terminal
        printf("\033[H\033[J"); // clear screen
        log_interface(&scan->interface);
        printf("\nScanning:\n");
        printf("ARP package send: ");
        
        for (uint8_t k = 0; k < 3; k++){
            printf("%u.", scan->arpPack.arpHeader.trgIp[k]);
        }
        printf("%u\n", scan->arpPack.arpHeader.trgIp[3]);
        i++;

        // delay between sending ARP requests
        usleep(scan->config->delay);
    } while (~(h | *(uint32_t *) mask) != 0);
    
    // waiting 2 seconds, after wait until listen Thread exit
    sleep(2);
    listenState = STOP_LISTEN;
    pthread_join(listenThreadId, NULL);
    printf("Scanning complete\n");
}

void send_arp_pack( Scanner *scan ) {
    int bytes;
    fd_set wfds;
    struct timeval tv;
    int select_res;
    
    // generate request
    FD_ZERO(&wfds);
    FD_SET(scan->sock_desc, &wfds);
    
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    select_res = select(scan->sock_desc + 1, NULL, &wfds, NULL, &tv);
    
    if (select_res == -1){
        perror("error with select usage");
        return;
    } else if (select_res == 0){
        printf("timout for sending ARP packet");
        return;
    }
    
    // send bytes to LAN
    if ((bytes = sendto(scan->sock_desc, &scan->arpPack, ARP_PACK_SIZE, 0, (struct sockaddr *) &scan->interface.device, sizeof(scan->interface.device))) < 0){
        perror("sending ARP packet");
        return;
    }
}

void *_start_listen(void *arg){
    Scanner *scan = (Scanner *) arg;
    int recv_sock;
    size_t bytes;
    ArpPack recvData;
    struct timeval timeout;

    socklen_t addrlen = (socklen_t) sizeof(struct sockaddr_ll);
    
    fd_set rfds;
    int select_res;
    
    // set timeout for listen ARP packets
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    FD_ZERO(&rfds);
    FD_SET(scan->sock_desc, &rfds);

    if ((recv_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0){
        perror("cannot create socket for receive ARP packets");
        return NULL;
    }
    
    listenState = START;
    while (listenState != STOP_LISTEN){
        // listen all ARP packets while main thread not change listenState variable to STOP_LISTEN
        select_res = select(recv_sock, &rfds, NULL, NULL, &timeout);
        if (select_res == -1){
            perror("select() for receiving socket");
            continue;
        } else if (select_res == 0){
            continue;
        }
        
        // receiving data
        memset(&recvData, 0, ARP_PACK_SIZE);
        
        if ((bytes = recvfrom(recv_sock, &recvData, ARP_PACK_SIZE, 0, (struct sockaddr *) &scan->interface.device, &addrlen) < 0)){
            perror("receive packet:");
        }
        
        if (*(uint32_t *) scan->interface.ipv4 == *(uint32_t *) recvData.arpHeader.trgIp && 
                                                                recvData.arpHeader.op == 0x0200
        ) {
            // add ip and mac addresses to Trie (Data structure) tree
            _add_addr(scan, recvData.arpHeader.srcIp, recvData.arpHeader.srcMac);
         }
    }
    
    // exit from listen thread
    pthread_exit(NULL);
}

void _add_addr(Scanner *scan, uint8_t *ip, uint8_t *mac){
    void **currAddr;
    if (scan->addrList == NULL)
        scan->addrList = calloc(256, sizeof(*scan->addrList));

    currAddr = scan->addrList;
    
    for (uint8_t i = 0; i < 3; i++){
        uint8_t ipNum = ip[i];

        if (currAddr[ipNum] == NULL){
            // adding IP part to heap memory
            currAddr[ipNum] = calloc(256, sizeof(void **));
        }

        currAddr = currAddr[ipNum];
    }

    if (currAddr[ ip[3] ] == NULL){
        // allocating memory from head for MAC address
        currAddr[ ip[3] ] = malloc(MAC_SIZE);
    }
    
    // writing MAC to the heap
    memcpy(currAddr[ ip[3] ], mac, MAC_SIZE);
}

void get_addrs(Scanner *scan){
    // magic terrible code :(
    void **pMem[5];         // Array of pointers
    uint16_t c[4] = {0};    // Array of counters
    uint8_t ip[4];
    uint8_t mac[6];
    uint8_t i = 0;
    if (scan->addrList == NULL){
        printf("LAN is silent...\n");
    }

    pMem[0] = scan->addrList;

    do {
        pMem[i + 1] = pMem[i] + c[i];
        if (*pMem[i + 1] != NULL){
            pMem[i + 1] = (void *) *(pMem[i + 1]);
            ip[i] = c[i];
            if (i > 2){
                // get ip and mac
                memcpy(mac, pMem[i + 1], sizeof(uint8_t) * 6);
                out_addr(scan, ip, mac);
            } else {
                c[i]++;
                i++;
                continue;
            }
        }

        c[i]++;
        if (c[i] > 255){
            c[i] = 0;
            i--;
        }
    } while (c[0] < 255);
}

static void _recur_clean(void **mem, uint8_t depth){
    if (depth <= 3){
        for (uint16_t i = 0; i < 256; i++){
            if(mem[i] != NULL)
                _recur_clean(mem[i], depth + 1);
        }
    }

    free(mem);
}

void out_addr(Scanner *scan, uint8_t *ip, uint8_t *mac){
    // output data (file or stdout)
    for (uint8_t i = 0; i < 3; i++){
        fprintf(scan->config->file, "%u.", ip[i]);
    }
    fprintf(scan->config->file, "%u\t->   ", ip[3]);

    for (uint8_t i = 0; i < 5; i++){
        fprintf(scan->config->file, "%02x:", mac[i]);
    }
    fprintf(scan->config->file, "%02x\n", mac[5]);
}

void clean_scanner(Scanner *scan){
    // clean memory garbage
    if (scan != NULL){
        if (scan->config != NULL){
            if (scan->config->file != stdout){
                fclose(scan->config->file);
            }
        }

        if (scan->addrList != NULL){
            _recur_clean(scan->addrList, 0);
        }
        
        free(scan);
    }
}
