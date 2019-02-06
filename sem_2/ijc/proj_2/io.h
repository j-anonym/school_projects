// io.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1


#ifndef PROJEKT2_IO_H
#define PROJEKT2_IO_H

#include <stdio.h>
#include <stdbool.h>

extern bool warning;

int get_word(char *s, int max, FILE *f);

#endif //PROJEKT2_IO_H
