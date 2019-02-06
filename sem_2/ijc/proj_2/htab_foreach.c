// wordcount.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include "htab.h"

void htab_foreach(htab_t *t, void (*func)(char *, unsigned long)) {

    if (t == NULL) {
        return;
    }
    for (unsigned long i = 0; i < htab_bucket_count(t); ++i) {

        for (struct htab_listitem *tmp = t->ptr[i]; tmp != NULL; tmp = tmp->next) {
            func(tmp->key, tmp->data);
        }
    }
}
