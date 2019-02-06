// htab_remove.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include "htab.h"

bool htab_remove(htab_t *t, const char* key) {

    if (t == NULL || key == NULL) {
        return false;
    }

    struct htab_listitem *previous;
    struct htab_listitem *next;

    unsigned long index = (htab_hash_function(key) % t->arr_size);

    previous = NULL;
    next = NULL;

    for (struct htab_listitem *tmp = t->ptr[index]; tmp != NULL; tmp = tmp->next) {
        next = tmp->next;

        if (strcmp(key, tmp->key) == 0) {
            t->size--;
            if (previous != NULL) {
                previous->next = next;
            }
            else {
                t->ptr[index] = next;
            }
            free(tmp->key);
            free(tmp);
            return true;
        }
        previous = tmp;
    }

    return false;
}

