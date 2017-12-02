#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_DIR "."
#define INPUT_BUF_SIZE 512

void make_fifo(const char* path) {
    if(mkfifo(path, S_IRUSR | S_IWUSR | S_IWGRP) == -1 &&
       errno != EEXIST) {
        fprintf(stderr, "Failed to create %s\n", path);
        exit(EXIT_FAILURE);
    }
}

int main() {
    char input[INPUT_BUF_SIZE];
    int inFd;
    int outFd = -1;
    const char inFifoPath[] = FIFO_DIR "/in.fifo";
    const char outFifoPath[] = FIFO_DIR "/out.fifo";

    umask(0);

    make_fifo(inFifoPath);
    make_fifo(outFifoPath);

    inFd = open(inFifoPath, O_RDONLY);
    if(inFd == -1) {
        fprintf(stderr, "in open() failed\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        ssize_t readSize = read(inFd, input, sizeof(input));
        if(readSize == -1) {
            fprintf(stderr, "failed to read\n");
            exit(EXIT_FAILURE);
        }
        if(readSize == 0) {
            break;
        }

        if(outFd == -1) {
            outFd = open(outFifoPath, O_WRONLY);
            if(outFd == -1) {
                fprintf(stderr, "out open() failed\n");
                exit(EXIT_FAILURE);
            }
        }

        if(write(outFd, input, readSize) != readSize) {
            fprintf(stderr, "failed to write\n");
            exit(EXIT_FAILURE);
        }
    }

    close(outFd);
    close(inFd);
    exit(EXIT_SUCCESS);
}
