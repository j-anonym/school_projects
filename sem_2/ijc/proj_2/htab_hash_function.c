// htab_hash_function.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include "htab.h"

unsigned int htab_hash_function(const char *str) {
    unsigned int h=0;     // 32bit
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
        h = 65599*h + *p;
    return h;
}