// wordcount.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdlib.h>
#include "htab.h"
#include "io.h"

bool warning = false;

#define MAX_LENGTH 127
#define SIZE 2048
// velkost som zvolil na zaklade clanku na wikipedii o hashovacich tabulkach
// (https://en.wikipedia.org/wiki/Hash_table)
// ak je hashovacia funkcia dobra, pracuje najlepsie ak velkost tabulky = 2^n


void print_function(char *string, unsigned long count) {
    printf("%s \t %lu\n",string,count);
}

int main (int argc, char *argv[]) {
    htab_t *htab = htab_init(SIZE);

    (void)argv;
    if (argc > 1) {
        fprintf(stderr, "ERROR: you should start program without parameters");
        return EXIT_FAILURE;
    }

    if (htab == NULL) {
        htab_free(htab);
        fprintf(stderr, "ERROR: fail to allocate memmory");
        return EXIT_FAILURE;
    }


    char word[MAX_LENGTH + 1] = {0};

    while (get_word(word, MAX_LENGTH, stdin) != EOF) {
        if (htab_lookup_add(htab,word) == NULL) {
            fprintf(stderr, "ERROR: fail to allocate memmory");
            return EXIT_FAILURE;
        }
    }

    htab_foreach(htab, print_function);
    htab_free(htab);


    if (warning) {
        fprintf(stderr, "WARNING: Maximum length of line is %d, your line will be shortened",MAX_LENGTH);
    }
    return 0;
}