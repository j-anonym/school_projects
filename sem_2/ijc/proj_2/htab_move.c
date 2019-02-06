// htab_move.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include "htab.h"

htab_t *htab_move(unsigned long newsize, htab_t *t2) {

    if (t2 == NULL) {
        return NULL;
    }

    htab_t *htab = htab_init(newsize);
    if (htab == NULL) {
        return NULL;
    }

    htab->size = t2->size;
    struct htab_listitem *tmp;

    for (unsigned long i = 0; i < htab_bucket_count(t2) ; ++i) {

        for (tmp = t2->ptr[i]; tmp != NULL;tmp = tmp->next) {

            unsigned long index = (htab_hash_function(tmp->key) % newsize);
            if (htab->ptr[index] == NULL) {
                htab->ptr[index] = tmp;
                tmp->next = NULL;
            }
            else {
                struct htab_listitem *first = htab->ptr[index];
                htab->ptr[index] = tmp;
                tmp->next = first;
            }
        }
        t2->ptr[i] = NULL;
    }
    t2->size = 0;
    return htab;
}