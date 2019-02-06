// htab_lookup_add.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include "htab.h"


struct htab_listitem *htab_lookup_add(htab_t *t, const char *key) {
    if (t == NULL || key == NULL) {
        return NULL;
    }

    unsigned long index = (htab_hash_function(key) % t->arr_size);

    bool found = false;

    struct htab_listitem *tmp;
    struct htab_listitem *lastItem = NULL;

    for (tmp = t->ptr[index]; tmp != NULL; tmp = tmp->next) {
        if (strcmp(key,tmp->key) == 0) {
            found = true;
            tmp->data++;
            break;
        }
        if (tmp->next == NULL) {
            lastItem = tmp;
        }
    }

    if (found) {
        return tmp;
    }

    struct htab_listitem *toBeAdded = malloc(sizeof(struct htab_listitem));

    if (toBeAdded == NULL) {
        return NULL;
    }

    char *string = malloc(sizeof(char) * (strlen(key) + 1));

    if (string == NULL) {
        free (toBeAdded);
        return NULL;
    }

    strcpy(string, key); // nastaveni klice v zaznamu


    toBeAdded->key = string;
    toBeAdded->data = 1;
    toBeAdded->next = NULL;

    if (lastItem == NULL) {
        t->ptr[index] = toBeAdded;
    }
    else {
        lastItem->next = toBeAdded;
    }

    t->size++;
    return toBeAdded;
}


