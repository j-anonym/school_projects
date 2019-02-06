// proj2.c
// Riesenie IOS-DU2, 29.4.2018
// Autor: Jan Vavro (xvavro05), FIT VUTBR
// Prelozene: gcc 7.3.1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <stdbool.h>
#include <zconf.h>

#define MAX_TIME 1000
int     *waiting = NULL, //number of riders waiting on bus stop
        *counter = NULL, //counter for program output
        *ridCounter = NULL, //counter for riders
        *transported = NULL; //counter for transported riders

int     waitingId = 0,
        counterId = 0,
        ridCounterId = 0,
        trasportedId = 0;

sem_t   *sem_mutex,
        *sem_mutex2,
        *sem_bus,
        *sem_boarded,
        *sem_can_finish;

FILE *f;

typedef struct { //structure for program arguments
    int r;
    int c;
    int art;
    int abt;
} arg_t;

/**
 * @brief process program arguments
 * @param argc number of arguments
 * @param argv array of arguments
 * @return all arguments in structure
 */
arg_t get_arg(int argc, char *argv[]);

/**
 * @brief count random number
 * @param max maximal number that can be returned
 * @return random number from <0,max>
 */
unsigned int random_number(unsigned int max);

/**
 * @brief function to create and initialize semaphores and shared memmory
 */
void set_resources();

/**
 * @brief function to erase semaphores and shared memmory
 */
void clean_resources();

/**
 * @brief function that perform bus operations
 * @param arguments input arguments of program
 */
void bus(arg_t arguments);

/**
 * @brief function that perform rider operations
 */
void rider();

int main(int argc, char *argv[]) {
    srand(time(NULL));

    arg_t arguments = get_arg(argc,argv);
    set_resources();
    setbuf(stdout, NULL);
    setbuf(f,NULL);
    setbuf(stderr, NULL);

    pid_t busPid = fork();

    if (busPid == 0){
        //child - bus
        bus(arguments);
    } else if (busPid > 0) {
        //parent - hlavny proces
        pid_t generatorPid = fork();
        if (generatorPid == 0) {
            //child - generator riderov
            for (int i = 0; i < arguments.r; i++) {
                usleep(random_number((unsigned int)arguments.art) * 1000);
                pid_t processPid = fork();

                if (processPid == 0) {
                    //child - rider
                    rider();
                } else if (processPid < 0){
                    fprintf(stderr, "ERROR: Fork failed.\n");
                    clean_resources();
                    return 1;
                }

            }
            exit(EXIT_SUCCESS);
        } else if (generatorPid < 0) {
            fprintf(stderr, "ERROR: Fork failed.\n");
            clean_resources();
            return 1;
        } else if (generatorPid > 0) {
            //hlavny proces, caka na skoncenie autobusu a generatoru
            waitpid(busPid, NULL, 0);
            waitpid(generatorPid, NULL, 0);
        }
    } else if (busPid < 0){
        fprintf(stderr, "ERROR: Fork failed.\n");
        clean_resources();
        return 1;
    }
    clean_resources();
    return 0;
}

arg_t get_arg(int argc, char *argv[]) {
    arg_t args;
    char *tmp;
    long number;
    bool error = false;

    if (argc != 5)
    {
        fprintf(stderr, "ERROR: Wrong count of arguments.\n");
        exit(1);
    }

    number = strtol(argv[1], &tmp, 10);
    if (number <= 0 || number > INT_MAX || *tmp != '\0')
        error = true;
    args.r = (int) number;

    number = strtol(argv[2], &tmp, 10);
    if (number <= 0 || number > INT_MAX || *tmp != '\0')
        error = true;
    args.c = (int) number;

    number = strtol(argv[3], &tmp, 10);
    if (number < 0 || number > MAX_TIME || *tmp != '\0')
        error = true;
    args.art = (int) number;

    number = strtol(argv[4], &tmp, 10);
    if (number < 0 || number > MAX_TIME || *tmp != '\0')
        error = true;
    args.abt = (int) number;

    if (error)
    {
        fprintf(stderr, "Chyba! Nespravne spusteni programu, neplatne hodnoty argumentu.\n");
        exit(1);
    }

    return args;
}

unsigned int random_number(unsigned int max) {
    if (max == 0)
        return 0;
    return rand() % max;
}

void set_resources() {
    bool error = false;

    if ((f = fopen("proj2.out", "w")) == NULL) {
        fprintf(stderr, "ERROR: Cannot open file for writing.\n");
        exit(EXIT_FAILURE);
    }


    /*-------------------SEMAFORY---------------------*/
    if ((sem_mutex = sem_open("xvavro05_mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
        error = true;

    if ((sem_mutex2 = sem_open("xvavro05_mutex2", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
        error = true;

    if ((sem_bus = sem_open("xvavro05_bus", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        error = true;

    if ((sem_boarded = sem_open("xvavro05_boarded", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        error = true;

    if ((sem_can_finish = sem_open("xvavro05_can_finish", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
        error = true;

    /*-------------------ZDIELANE-PREMENNE-------------*/
    if ((waitingId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        error = true;
    if ((waiting = shmat(waitingId, NULL, 0)) == NULL)
        error = true;
    if ((counterId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        error = true;
    if ((counter = shmat(counterId, NULL, 0)) == NULL)
        error = true;
    if ((trasportedId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        error = true;
    if ((transported = shmat(trasportedId, NULL, 0)) == NULL)
        error = true;
    if ((ridCounterId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
        error = true;
    if ((ridCounter = shmat(ridCounterId, NULL, 0)) == NULL)
        error = true;

    /*-------------------NASTAVENIE-HODNOT--------------*/
    *waiting = 0;
    *counter = 0;
    *ridCounter = 0;
    *transported = 0;
    /*------------------Something went wrong-----------*/
    if (error)
    {
        fprintf(stderr, "ERROR: Cannot create semaphore or allocate shared memmory.\n");
        clean_resources();
        exit(EXIT_FAILURE);
    }
}

void clean_resources () {
    bool error = false;

    if (fclose(f) == EOF) {
        fprintf(stderr, "ERROR: Cannot close file.\n");
        exit(EXIT_FAILURE);
    }
    /*------------------SEMAFORY---------------------*/
    sem_close(sem_mutex);
    sem_close(sem_mutex2);
    sem_close(sem_bus);
    sem_close(sem_boarded);
    sem_close(sem_can_finish);


    sem_unlink("xvavro05_mutex");
    sem_unlink("xvavro05_mutex2");
    sem_unlink("xvavro05_bus");
    sem_unlink("xvavro05_boarded");
    sem_unlink("xvavro05_can_finish");


    /*------------------ZDIELANE-PREMENNE------------*/
    if (shmctl(waitingId, IPC_RMID, NULL) == -1)
        error = true;
    if (shmctl(trasportedId, IPC_RMID, NULL) == -1)
        error = true;
    if (shmctl(waitingId, IPC_RMID, NULL) == -1)
        error = true;
    if (shmctl(ridCounterId, IPC_RMID, NULL) == -1)
        error = true;


    if (error)
    {
        fprintf(stderr, "ERROR: Cannot free shared memmory.\n");
        exit(EXIT_FAILURE);
    }

}

void bus(arg_t arguments) {
    sem_wait(sem_mutex2);
    fprintf(f,"%-10d: %-10s: start\n",++(*counter),"BUS");
    sem_post(sem_mutex2);
    while ((*transported) != arguments.r ) {

        sem_wait(sem_mutex);
        sem_wait(sem_mutex2);
        fprintf(f,"%-10d: %-10s: arrival\n",++(*counter),"BUS" );
        sem_post(sem_mutex2);
        int n;
        if (*waiting > arguments.c)
            n = arguments.c;
        else
            n = *waiting;

        if (*waiting == 0) {
            sem_post(sem_mutex);
            sem_wait(sem_mutex2);
            fprintf(f,"%-10d: %-10s: depart\n",++(*counter),"BUS" );
            sem_post(sem_mutex2);
            usleep(random_number((unsigned int)arguments.abt) * 1000);
            sem_wait(sem_mutex2);
            fprintf(f,"%-10d: %-10s: end\n",++(*counter),"BUS" );
            sem_post(sem_mutex2);

            continue;
        }

        sem_wait(sem_mutex2);
        fprintf(f,"%-10d: %-10s: start boarding: %d\n",++(*counter),"BUS",(*waiting));
        sem_post(sem_mutex2);

        for (int i = 0; i < n; i++) {
            sem_post(sem_bus);
            sem_wait(sem_boarded);
        }

        int tmp = *waiting - arguments.c;
        if (tmp > 0)
            *waiting = tmp;
        else
            *waiting = 0;

        sem_wait(sem_mutex2);
        fprintf(f,"%-10d: %-10s: end boarding: %d\n",++(*counter),"BUS",(*waiting));
        sem_post(sem_mutex2);

        sem_wait(sem_mutex2);
        fprintf(f,"%-10d: %-10s: depart\n",++(*counter),"BUS" );
        sem_post(sem_mutex2);
        sem_post(sem_mutex);
        usleep(random_number((unsigned int)arguments.abt) * 1000);

        sem_wait(sem_mutex2);
        fprintf(f,"%-10d: %-10s: end\n",++(*counter),"BUS" );
        sem_post(sem_mutex2);

        for (int i = 0; i < n; i++) {
            sem_post(sem_can_finish);
        }
    }

    sem_wait(sem_mutex2);
    fprintf(f,"%-10d: %-10s: finish\n",++(*counter),"BUS");
    sem_post(sem_mutex2);
    exit(EXIT_SUCCESS);
}

void rider() {
    int ridNumber;
    sem_wait(sem_mutex2);
    ridNumber = ++(*ridCounter);
    fprintf(f,"%-10d: %s %-6d: start\n",++(*counter),"RID",ridNumber);
    sem_post(sem_mutex2);

    sem_wait(sem_mutex);
    (*waiting) += 1;
    sem_wait(sem_mutex2);
    fprintf(f,"%-10d: %s %-6d: enter: %d\n",++(*counter),"RID",ridNumber, (*waiting));
    sem_post(sem_mutex2);
    sem_post(sem_mutex);

    sem_wait(sem_bus);
    sem_wait(sem_mutex2);
    fprintf(f,"%-10d: %s %-6d: boarding\n",++(*counter),"RID",ridNumber);
    sem_post(sem_mutex2);

    sem_post(sem_boarded);

    sem_wait(sem_mutex);
    (*transported) += 1;
    sem_post(sem_mutex);

    sem_wait(sem_can_finish);
    sem_wait(sem_mutex2);
    fprintf(f,"%-10d: %s %-6d: finish\n",++(*counter),"RID",ridNumber);
    sem_post(sem_mutex2);
    exit(EXIT_SUCCESS);
}
