#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main() {
    const char shm_name[] = "/test.shm";
    size_t shm_size = 1* 1024*1024;
    void* addr;
    int shm_fd;

    shm_fd = shm_open(shm_name, O_CREAT|O_RDWR, 0666);
    if(shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm_fd, shm_size) == -1) {
        perror("fturncate");
        exit(EXIT_FAILURE);
    }

    addr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memset(addr, 13, shm_size);

    exit(EXIT_SUCCESS);
}
