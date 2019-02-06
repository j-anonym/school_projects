// htab_init.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdlib.h>
#include "htab.h"

htab_t *htab_init(unsigned long size) {

    htab_t *htab = malloc(sizeof(htab_t) + size * sizeof(struct htab_listitem *));

    if (htab == NULL) {
        return NULL;
    }

    htab->arr_size = size;
    htab->size = 0;

    for (unsigned long i = 0; i < size; ++i) {
        htab->ptr[i] = NULL;
    }

    return htab;
}