// steg-decode.c
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#include "eratosthenes.h"
#include "ppm.h"
#include <stdio.h>
#include <stdlib.h>

#define LIMIT 3000000 //limit velkosti vstupnych dat
/**
 * @brief funkce dekoduje spravu zakodovanu v obrazku (vid http://www.fit.vutbr.cz/study/courses/IJC/public/DU1.html.cs)
 * @return 0 pri uspechu
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        warning_msg("Nespravne zadane argumenty\n");
        return -1;
    }

    struct ppm *image = ppm_read(argv[1]);
    if (image == NULL) {
        error_exit("Chyba pri citani ze souboru %s\n",argv[1]);
    }
    bit_array_create(bitArray, LIMIT);
    Eratosthenes(bitArray);
    int bitCount = 0;
    char bit,result = 0;

    for (int i = 11; i < LIMIT; i++) {
        if ( bit_array_getbit(bitArray,i) == 0) {
            bit = ((image->data[i]) & ((char) 1));
            bit <<= (bitCount);
            result |= bit;
            bitCount++;
            if (bitCount == 8) {
                printf("%c", result);
                bitCount = 0;
                if (result == '\0') {
                    printf("\n");
                    break;
                }
                result = 0;
            }
        }
        if (i == LIMIT - 1) {
            warning_msg("Dekodovana sprava nebola korektne ukoncena");
        }
    }
    free(image);

    return 0;
}