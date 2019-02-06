// htab_remove.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdlib.h>
#include "htab.h"

void htab_clear(htab_t *t) {
    if (t == NULL) {
        return;
    }
    for (unsigned long i = 0; i < htab_bucket_count(t); ++i) {

        for (struct htab_listitem *tmp = t->ptr[i]; tmp != NULL; tmp = tmp->next) {
            free(tmp->key);
            free(tmp);
        }
        t->ptr[i] = NULL;
    }
    t->size = 0;
}