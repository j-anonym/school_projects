// eratosthenes.h
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#ifndef ERATOSTHENES_H
#define ERATOSTHENES_H

#include "bit_array.h"
/**
 * @brief funkce nastavi na prvociselnych indexech bitoveho pola zadaneho parametrem hodnotu 0
 * @param bitArray bitove pole ve kterem hledame prvocisla
 */
void Eratosthenes(bit_array_t bitArray);
#endif //ERATOSTHENES_H
