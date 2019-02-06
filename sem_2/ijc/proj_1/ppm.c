// ppm.c
// Řešení IJC-DU1, příklad a), 14.3.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Přeloženo: gcc 7.3.1

#include "ppm.h"
#include <stdio.h>
#include <stdlib.h>

struct ppm * ppm_read(const char * filename) {
    struct ppm *picture;
    unsigned x,y;
    FILE *f = fopen(filename,"r");

    if (f == NULL) {
        warning_msg("Chyba pri otevirani souboru\n");
        return NULL;
    }

    if (fscanf(f,"P6 %u %u 255 ", &x, &y) != 2) {
        warning_msg("Nespravny format hlavicky souboru %s\n",filename);
        return NULL;
    }

    unsigned long memory = 3 * x * y * sizeof(char);

    picture = malloc(memory + sizeof(struct ppm));
    if (picture == NULL) {
        warning_msg("Pamet na halde sa nezdarilo alokovat\n");
        fclose(f);
        return NULL;
    }

    if (fread(picture->data, sizeof(char), memory, f) != memory) {
        warning_msg("Nevalidny obsah binarnych dat\n");
        fclose(f);
        free(picture);
        return NULL;
    }

    picture->xsize = x;
    picture->ysize = y;
    fclose(f);
    return picture;

}

int ppm_write(struct ppm *p, const char * filename) {

    FILE *f = fopen(filename,"w");

    if (f == NULL) {
        warning_msg("Chyba pri otevirani souboru\n");
        return -1;
    }
    fprintf(f,"P6 %u %u 255 ", p->xsize, p->ysize);

    unsigned long memory = 3 * p->xsize * p->ysize * sizeof(char);

    fwrite(p->data, sizeof(char), memory, f);
    free(p);
    fclose(f);

    return EXIT_SUCCESS;
}