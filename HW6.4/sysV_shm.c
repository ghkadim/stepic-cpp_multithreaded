#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>

int main() {
    key_t key;
    int shm_id;
    void* addr;
    size_t shm_size = 1*1024*1024;

    key = ftok("/tmp/mem.temp", 1);
    if(key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    shm_id = shmget(key, shm_size, IPC_CREAT | 0666);
    if(shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    addr = shmat(shm_id, NULL, 0);
    if(addr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    memset(addr, 42, shm_size);

    exit(EXIT_SUCCESS);
}
