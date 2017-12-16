#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define HANDLE_ERROR( MSG, ... ) \
    do { \
        perror(MSG); \
        __VA_ARGS__; \
        return EXIT_FAILURE; \
    } while(0)

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
    struct seminfo * __buf;
};

int main() {
    int semid;
    unsigned short sem_values[16];
    union semun arg;

    key_t key = ftok("/tmp/sem.temp", 1);
    if(key == -1) {
        HANDLE_ERROR("ftok");
    }

    for(int i=0; i<16; ++i) {
        sem_values[i] = i;
    }

    semid = semget(key, 16, IPC_CREAT | 0666);
    if(semid == -1) {
        HANDLE_ERROR("semget");
    }

    arg.array = sem_values;
    if(semctl(semid, 0, SETALL, arg) == -1) {
        HANDLE_ERROR("semctl");
    }

    exit(EXIT_SUCCESS);
}
