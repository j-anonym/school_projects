// tail.c
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief print usage of function tail
 */
void usage();

/**
 * @brief function read and print last numberOfLines from file named f
 * @param f name of file or stdin
 * @param numberOfLines number of last n lines to be read and printed
 */
void read_and_write(char *f, long numberOfLines);

/**
 * @brief function proccess program input
 * @param argc number of arguments
 * @param argv matrix with arguments
 * @param numberOfLines pointer to number of lines to be printed
 * @param file pointer to namefile
 */
int get_option(int argc, char **argv, long *numberOfLines, char **file);

#define MAX_LENGTH 1024
#define WITHOUT_PARAMETERS_STDIN 1
#define ONE_PARAM_FILE 2
#define TWO_PARAM_STDIN 3
#define THREE_PARAM_FILE 4
#define BAD_ARGUMENTS (-1)

int main (int argc, char *argv[]) {

    long numberOfLines = 10;
    char *file = "stdin";
    int option = get_option(argc,argv, &numberOfLines, &file);

    if (option == BAD_ARGUMENTS) {
        fprintf(stderr,"ERROR: Wrong arguments.\n");
        usage();
        return BAD_ARGUMENTS;
    }
    else {
        read_and_write(file,numberOfLines);
    }

    return 0;
}

void read_and_write(char *f, long numberOfLines){

    int i = 0, c;
    int charsCount = 0;
    bool warning = false;
    char buffer[numberOfLines][MAX_LENGTH + 2];
    memset(buffer, 0, sizeof(buffer[0][0]) * numberOfLines * (MAX_LENGTH + 2));
    FILE *file;

    if (strcmp(f,"stdin") != 0) {

        file = fopen(f, "r");
        if (file == NULL) {
            fprintf(stderr, "ERROR: Cannot open file for reading.\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        file = stdin;
    }

    if (numberOfLines == 0) {
        return;
    }
    while((c = getc(file)) != EOF) {
        buffer[i % numberOfLines][charsCount] = (char)c;
        charsCount++;
        if (c == '\n') {
            i++;
            charsCount = 0;
        }
        if (charsCount == MAX_LENGTH + 1) {
            warning = true;
            buffer[i % numberOfLines][MAX_LENGTH] = '\n';
            while (getc(file) != '\n');
            charsCount = 0;
            i++;
        }
    }
    if (strcmp(f,"stdin") != 0) {
        if (fclose(file) == EOF) {
            fprintf(stderr, "ERROR: Cannot close file.\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int k = 0; k < numberOfLines; k++,i++) {
        for (int j = 0; j <= MAX_LENGTH; j++) {
            if (buffer[i % numberOfLines][j-1] == '\n')
                break;
            printf("%c",buffer[i % numberOfLines][j]);
        }
    }
    if (warning) {
        fprintf(stderr,"WARNING: Maximum length of line (%d) was exceeded, line will be shortened.\n",MAX_LENGTH);
    }
}

int get_option(int argc, char **argv, long *numberOfLines, char **file) {
    if (argc == 1) {
        return WITHOUT_PARAMETERS_STDIN;
    }
    else if (argc == 2) {
        *file = argv[1];
        return ONE_PARAM_FILE;
    }
    else if (argc == 3 || argc == 4) {
        if (strcmp(argv[1],"-n") == 0) {
            char *tmp;
            *numberOfLines = strtol(argv[2], &tmp, 10);
            if (strcmp(tmp,"\0") == 0 && (*numberOfLines) >= 0) {
                if (argc == 3) {
                    return TWO_PARAM_STDIN;
                }
                else {
                    *file = argv[3];
                    return THREE_PARAM_FILE;
                }
            }
        }
    }
    return -1;
}

void usage() {
    printf("USAGE:\ntail [-n number] filename\nor\ntail [-n number] <filename\nMaximum length of line is %d chars.\n",MAX_LENGTH);
}