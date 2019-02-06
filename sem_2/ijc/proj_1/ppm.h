// error.h
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1
#ifndef PPM_H
#define PPM_H

#include "error.h"
/**
 * @brief struktura reprezentujici obsah ppm suboru
 */
struct ppm {
    unsigned xsize;
    unsigned ysize;
    char data[];    // RGB bajty, celkem 3*xsize*ysize
};
/**
 * @biref funkce spracuje obrazek v ppm formatu
 * @param filename nazev ppm souboru umistneneho v zlozce projektu
 * @return funkce vraci ukazatel na strukturu v ktorej su rozmery obrazku a binarni RGB data
 */
struct ppm * ppm_read(const char * filename);

/**
 * @brief funkce vytvori obrazek ve formate ppm ze zadanych dat
 * @param p ukazatel na strukturu obsahujuci data obrazku
 * @param filename nazev vystupnyho souboru
 * @return 0 pri uspechu
 * @return -1 pri neuspechu
 */
int ppm_write(struct ppm *p, const char * filename);
#endif //PPM_H
