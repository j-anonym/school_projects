// htab_remove.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdlib.h>
#include "htab.h"

void htab_free(htab_t *t) {
    htab_clear(t);
    free(t);
    t = NULL;
}
