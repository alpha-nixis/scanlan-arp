#include <stdio.h>          // printf
#include <stdint.h>         // uint8_t, uint16_t ...
#include <stdlib.h>         // exit(), EXIT_SUCCESS EXIT_FAILURE
#include <errno.h>          // errno, perror()
#include <unistd.h>         // getopt()
#include <math.h>           // pow()
#include <pthread.h>

#include "scanner.h"

#define OPTSTRING       "hH?i:I:f:F:d:D:"

#define DEFAULT_DELAY   ((useconds_t) 2 * powl(10, 5))

void print_usage( void );

int main(int argc, char *argv[]){
    Scanner *scan;
    Conf config = {.interface_name = NULL, .file = stdout, .delay = DEFAULT_DELAY};
    int opt;
    
    // arguments parsing
    while ((opt = getopt(argc, argv, OPTSTRING)) != -1){
        switch(opt){
            case 'h':
            case 'H':
            case '?':
                print_usage();
                exit(EXIT_SUCCESS);
                break;

            case 'i':
            case 'I':
                config.interface_name = (uint8_t *) optarg;
                break;

            case 'f':
            case 'F':
                config.file = fopen(optarg, "w");
                if (config.file == NULL){
                    perror("File");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'd':
            case 'D':
                config.delay = (useconds_t) atol(optarg);
                if (config.delay == 0){
                    fprintf(stderr, "Invalid time delay argument %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

                break;

        }
    }

    
    // create scanner
    scan = create_scanner(&config);
    if (scan == NULL){
        fprintf(stderr, "Scanner not create. Program aborted\n");
        exit(EXIT_FAILURE);
    }
    
    if (errno != 0){
        fprintf(stderr, "Aborted scanning process\n");
    } else {
        // sending ARP packet
        start_scan(scan);
        get_addrs(scan);
    }
    
    clean_scanner(scan);
    return EXIT_SUCCESS;
}

void print_usage( void ){
    printf("Usage: sudo ./scanlan-arp [ -hH? ] [ -i INTERFACE ] [-f FILE ] [-d DELAY ] (in root privileges)\n");
    printf("\t-h -H -?\tDisplay this menu\n");
    printf("\t-i -I INTERFACE\tSpecifying interface name, which will be used for scanning. By default first interface which is UP and RUNNING will be used.\n");
    printf("\t-f -F FILE\tWrite output data in file. By deafult output data will be write in terminal\n");
    printf("\t-d -D DELAY\tSpecifying delay (uSeconds) between sending ARP packets (writing data in socket). By default 100 milliseconds\n");
}
