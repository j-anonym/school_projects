// htab_size.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <glob.h>
#include "htab.h"

size_t htab_size(htab_t *t) {
    return t->size;
}