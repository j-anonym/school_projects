// htab_find.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdbool.h>
#include <memory.h>
#include "htab.h"

struct htab_listitem *htab_find(htab_t *t, const char *key) {
    if (t == NULL || key == NULL) {
        return NULL;
    }

    unsigned long index = (htab_hash_function(key) % t->arr_size);

    bool found = false;

    struct htab_listitem *tmp;

    for (tmp = t->ptr[index]; tmp != NULL; tmp = tmp->next) {
        if (strcmp(key,tmp->key) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        return tmp;
    }

    return NULL;
}