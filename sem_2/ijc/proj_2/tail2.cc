// tail2.cc
// Riesenie IJC-DU2, příklad a), 20.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
using namespace std;

/**
 * @brief function proccess program input
 * @param argc number of arguments
 * @param argv matrix with arguments
 * @param numberOfLines pointer to number of lines to be printed
 * @param file pointer to namefile
 */
void get_option(int argc, char **argv, long *numberOfLines, string *file);

/**
 * @brief function read and print last numberOfLines from file named f
 * @param f name of file or stdin
 * @param numberOfLines number of last n lines to be read and printed
 */
void read_and_write(string file,unsigned long numberOfLines);

int main (int argc, char *argv[]) {

    ios::sync_with_stdio(false);

    long numberOfLines = 10;
    string file = "stdin";
    get_option(argc, argv, &numberOfLines, &file);

    if (numberOfLines == -1) {
        fprintf(stderr,"ERROR: Wrong arguments.\n");
        return EXIT_FAILURE;
    }

    read_and_write(file,(unsigned long)numberOfLines);

    return 0;
}

void read_and_write(string file,unsigned long numberOfLines) {

    istream *in;
    ifstream f;

    if (file.compare("stdin") == 0) {
        in = &cin;
    }
    else {
        f.open(file, ios::in);
        if (!(f.is_open())) {
            cerr << "ERROR: Cannot open file for reading." << endl;
            exit(EXIT_FAILURE);
        }
        in = &f;
    }

    queue<string> buffer;
    string line;

    while (getline(*in, line)) {
        buffer.push(line);

        if (buffer.size() > numberOfLines) {
            buffer.pop();
        }
    }

    numberOfLines = buffer.size();

    for (unsigned long i = 0; i < numberOfLines; i++) {
        cout << buffer.front() << endl;
        buffer.pop();
    }

    f.close();
}


void get_option(int argc, char **argv, long *numberOfLines, string *file) {
    if (argc == 1) {
        return;
    }
    else if (argc == 2) {
        *file = argv[1];
        return;
    }
    else if (argc == 3 || argc == 4) {
        if (strcmp(argv[1],"-n") == 0) {
            char *tmp;
            *numberOfLines = strtol(argv[2], &tmp, 10);
            if (strcmp(tmp,"\0") == 0 && (*numberOfLines) >= 0) {
                if (argc == 3) {
                    return;
                }
                else {
                    *file = argv[3];
                    return;
                }
            }
        }
    }
    *numberOfLines = -1;
    return;
}
