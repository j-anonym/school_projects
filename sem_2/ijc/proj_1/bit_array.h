// bit_array.h
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#include <limits.h>
#include <stdbool.h>
#include "error.h"

#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

/**
 * @brief pole unsigned long, fungujici jako bitove pole
 */
typedef unsigned long bit_array_t[];

/**
 * @brief makro vytvori bitove pole
 * @param jmeno_pole nazev bitoveho pole
 * @param velikost pocet bitov v bitovom poli
 */
#define bit_array_create(jmeno_pole,velikost) unsigned long jmeno_pole[(velikost / sizeof(unsigned long) * CHAR_BIT) + ((velikost % sizeof(unsigned long) * CHAR_BIT) ? 2 : 1)] = {velikost, 0,}

#ifdef USE_INLINE

/**
 * @param jmeno_pole nazov bitoveho pole
 * @return funkce vraci pocet bitu v bitvem poli
 */
static inline unsigned long bit_array_size(bit_array_t jmeno_pole) {
    return jmeno_pole[0];
}

#else

#define bit_array_size(jmeno_pole) jmeno_pole[0]

#endif //USE_INLINE

#ifdef USE_INLINE
/**
 * @brief funkce nastavi bit na danem indexe na hodnotu 0 alebo 1
 * @param jmeno_pole nazev bitoveho pole
 * @param index urcuje na kterem bite se nastavi hodnota
 * @param vyraz dle ktereho se nastavi bit na 1 alebo 0
 */
static inline void bit_array_setbit(bit_array_t jmeno_pole, unsigned long index, bool vyraz) {
    unsigned long mez = bit_array_size(jmeno_pole);

    if (index < 0 || index >= mez)
        error_exit("Index %lu mimo rozsah 0..%lu", index, mez);

    if(vyraz)
        jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] |= ((unsigned long)1 << (index % (sizeof(*jmeno_pole) * CHAR_BIT)));
    else
        jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] &= ~((unsigned long)0 << (index % (sizeof(*jmeno_pole) * CHAR_BIT)));
}

#else

#define bit_array_setbit(jmeno_pole,index,vyraz)\
    do {\
        unsigned long mez = bit_array_size(jmeno_pole);\
        if (index < 0 || index >= mez)\
        error_exit("Index %lu mimo rozsah 0..%lu", index, mez);\
        \
        if(vyraz)\
            jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] |= ((unsigned long)1 << (index % (sizeof(*jmeno_pole) * CHAR_BIT)));\
        else\
            jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] &= ~((unsigned long)0 << (index % (sizeof(*jmeno_pole) * CHAR_BIT)));\
    } while(0)
#endif //USE_INLINE

#ifdef USE_INLINE
/**
 * @param jmeno_pole nazev bitoveho pole
 * @param index index bitu ktereho hodnutu zistujeme
 * @return funkce vraci hodnotu bitu (0/1) na danem indexu
 */
static inline bool bit_array_getbit(bit_array_t jmeno_pole, unsigned long index) {
    unsigned long mez = bit_array_size(jmeno_pole);

    if (index < 0 || index >= mez)
        error_exit("Index %lu mimo rozsah 0..%lu", index, mez);

    if (jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] & ((unsigned long)1 << (index % (sizeof(*jmeno_pole) * CHAR_BIT))))
        return 1;
    else
        return 0;
}

#else

#define bit_array_getbit(jmeno_pole,index)\
    (index < 0 || index >= bit_array_size(jmeno_pole))?\
        error_exit("Index %lu mimo rozsah 0..%lu", index, bit_array_size(jmeno_pole)-1), 0\
    :\
        (bool)(jmeno_pole[index / (sizeof(*jmeno_pole) * CHAR_BIT) + 1] & ((unsigned long)1 << (index % (sizeof(*jmeno_pole) * CHAR_BIT))))
#endif //USE_INLINE

#endif //BIT_ARRAY_H
