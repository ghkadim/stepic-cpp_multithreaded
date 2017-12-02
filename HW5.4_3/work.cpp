#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int sv[2];
    int pid;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        exit(1);
    }
    switch ((pid = fork())) {
    case 0:
        close(sv[0]);
        pause();
        break;
    case -1:
        perror("fork");
        exit(1);
    default: {
        int pid;
        close(sv[1]);
        wait(&pid);
        break;
    }
    }
    return 0;
}
