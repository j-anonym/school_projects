// primes.c
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1
#include <stdio.h>
#include "eratosthenes.h"
/**
 * @brief funkce vypise v zestupnem poradi poslednych 10 prvocisel mensich jako cislo dane makrem LIMIT
 * @return 0 pri uspechu
 */
#define LIMIT 222000000
int main() {
    bit_array_create(bitArray, LIMIT);
    Eratosthenes(bitArray);
    unsigned long vyslednePrvocisla[10] = {0,};
    int i = 0;

    for (unsigned long k = LIMIT - 1; k > 0; k--) {
        if (bit_array_getbit(bitArray,k) == 0) {
            vyslednePrvocisla[i] = k;
            i++;
        }
        if (i == 10)
            break;
    }

    for (int j = 9; j >= 0; j--) {
        printf("%lu\n", vyslednePrvocisla[j]);
    }

    return 0;
}
