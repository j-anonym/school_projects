// htab_size.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include "htab.h"

size_t htab_bucket_count(htab_t *t) {
    return t->arr_size;
}