#include "String.h"

uint8_t strequal(uint8_t *str_1, uint8_t *str_2)
{
    uint32_t i = 0;

    while( 1 ){
        if (str_1[i] != str_2[i])
            return 0;

        if (str_1[i++] == 0)
            return 1;
    }
}
