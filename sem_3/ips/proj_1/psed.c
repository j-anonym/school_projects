// Autori: Martin Buchta (xbucht28) a Jan Vavro (xvavro05)

#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <string.h>
#include <regex>


std::vector<std::mutex *> zamky; /* pole zamku promenne velikosti */
std::mutex *stdOutMutex; // zamek pro tisk na vystup
std::mutex *threadIdMutex; // zamek pro promennou threadIdDone

char *line;
int threadIdDone;

char *to_cstr(std::string a) {
    // prevede retezec v c++ do retezce v "c" (char *)
    char *str;
    str = (char *) malloc(sizeof(char) * (a.length() + 1));
    strcpy(str, a.c_str());
    return str;
}

char *read_line(int *res) {
    std::string line;
    char *str;
    if (std::getline(std::cin, line)) {
        str = to_cstr(line);
        *res = 1;
        return str;
    } else {
        *res = 0;
        return NULL;
    }
}


void f(int ID, char *search, char *replace) {
    /* funkce implementujici thread */
    char *result;
    std::regex reg(search);

    while (1) {
        zamky[ID]->lock();

        // skonci, kdyz nic neni na vstupu
        if (line == NULL) {
            free(result);
            result = NULL;
            return;
        }

        result = to_cstr(std::regex_replace(line, reg, replace));

        // aktivni cekani
        // cekej, dokud dane vlakno neni na rade na vypis vystupu
        threadIdMutex->lock();
        while (threadIdDone != ID) {
            threadIdMutex->unlock();
            threadIdMutex->lock();
        }
        threadIdMutex->unlock();

        // vytiskni vysledek
        stdOutMutex->lock();
        printf("%s\n", result);
        stdOutMutex->unlock();

        free(result);
        result = NULL;

        threadIdMutex->lock();
        threadIdDone++;
        threadIdMutex->unlock();
    }

}

int main(int argc, char *args[]) {
    // kontrola vstupu
    if ((argc) % 2 == 0 || argc < 3) {
        fprintf(stderr, "Chybný počet argumentů!\n");
        return 1;
    }

    /*******************************
     * Inicializace threadu a zamku
     * *****************************/
    int regexCount = (argc - 1) / 2;
    int num = regexCount;
    std::vector<std::thread *> threads; /* pole threadu promenne velikosti */

    /* vytvorime zamky */
    zamky.resize(regexCount); /* nastavime si velikost pole zamky */
    for (int i = 0; i < regexCount; i++) {
        std::mutex *new_zamek = new std::mutex();
        new_zamek->lock();
        zamky[i] = new_zamek;
    }

    // zamek pro stdout
    stdOutMutex = new std::mutex();

    // zamek pro threadId
    threadIdMutex = new std::mutex();

    /* vytvorime thready */
    threads.resize(regexCount); /* nastavime si velikost pole threads */
    for (int i = 0; i < regexCount; i++) {
        std::thread *new_thread = new std::thread(f, i, args[2 * i + 1], args[2 * i + 2]);
        threads[i] = new_thread;
    }
    /**********************************
     * Vlastni vypocet psed
     * ********************************/
    int res;
    line = read_line(&res);
    while (res) {
        threadIdMutex->lock();
        threadIdDone = 0;
        threadIdMutex->unlock();

        for (int i = 0; i < regexCount; i++) {
            zamky[i]->unlock();
        }

        // aktivni cekani
        // cekej, dokud vsechna vlakna nevytisknou vysledek radku
        threadIdMutex->lock();
        while (threadIdDone != regexCount) {
            threadIdMutex->unlock();
            threadIdMutex->lock();
        }
        threadIdMutex->unlock();

        free(line); /* uvolnim pamet */
        line = NULL;
        line = read_line(&res);
    }

    // probud vsechna vlakna, ta se ukonci

    for (int i = 0; i < regexCount; i++) {
        zamky[i]->unlock();
    }


    /**********************************
     * Uvolneni pameti
     * ********************************/

    /* provedeme join a uvolnime pamet threads */
    for (int i = 0; i < num; i++) {
        threads[i]->join();
        delete threads[i];
        threads[i] = NULL;
    }
    /* uvolnime pamet zamku */
    for (int i = 0; i < regexCount; i++) {
        delete zamky[i];
        zamky[i] = NULL;
    }

    delete stdOutMutex;
    stdOutMutex = NULL;

    delete threadIdMutex;
    threadIdMutex = NULL;

    free(line);
    line = NULL;
}
