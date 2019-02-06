// io.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "io.h"

int get_word(char *s, int max, FILE *f) {

    if (f == NULL) {
        return EOF;
    }

    int c;
    int i = 0;

    while ((c = getc(f)) != EOF) {
        if (i == 0 && isspace(c)) {
            while (isspace(c = getc(f)));
            ungetc(c,f);
        }
        if (isspace(c)) {
            s[i] = '\0';
            return i;
        }
        if (i == max) {
            while (!isspace(getc(f)));
            warning = true;
            s[i] = '\0';
            return i;
        }
        s[i]= c;
        i++;
    }
    return EOF;
}