// eratosthenes.c
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#include "eratosthenes.h"
#include <math.h>

void Eratosthenes(bit_array_t pole) {
    unsigned long velikost = bit_array_size(pole);
    bit_array_setbit(pole, 0, 1);
    bit_array_setbit(pole, 1, 1);

    for (unsigned long i = 2; i < (sqrt(velikost) + 1); i++) {
        if (bit_array_getbit(pole, i) == 0) {
            for (unsigned long j = 2; i * j < velikost; j++) {
                bit_array_setbit(pole, i * j, 1);
            }
        }
    }
}

