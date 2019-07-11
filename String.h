#ifndef __STRING_H
#define __STRING_H

#include <string.h>
#include <stdint.h>

#define Strcmp(arr_first, arr_second) strcmp((char *) (arr_first), (char *) (arr_second))
#define Strlen(str_) 		      strlen((char *) (str_))
#define Sprintf(arr_, f_, ...)	      sprintf((char *)(arr_), (f_), ##__VA_ARGS__)
#define Sscanf(str_, f_, ...)         sscanf( (char *)(str_), (f_), ##__VA_ARGS__)
#define Strequal(str1_, str2_)        strequal( (uint8_t *)(str1_), (uint8_t *)(str2_) )

uint8_t strequal(uint8_t *, uint8_t *);

#endif
