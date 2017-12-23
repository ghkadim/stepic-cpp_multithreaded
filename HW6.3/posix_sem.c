#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

int main() {
    const char sem_name[] = "/test.sem";
    sem_t* sem = NULL;

    sem = sem_open(sem_name, O_CREAT, 0666, 66);
    if(sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
