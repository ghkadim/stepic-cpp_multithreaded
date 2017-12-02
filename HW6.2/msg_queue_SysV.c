#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>

struct message {
    long mtype;
    char mtext[80];
};

int main() {
    int fd;
    const char fileName[] = "/home/box/message.txt";
    int readLen;
    int queueId;
    struct message msg;

    key_t key = ftok("/tmp/msg.temp", 1);
    if(key == -1) {
        perror("ftok failed\n");
        exit(EXIT_FAILURE);
    }

    queueId = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(queueId == -1) {
        perror("msgget failed\n");
        exit(EXIT_FAILURE);
    }

    readLen = msgrcv(queueId, &msg, 80, 0, 0);
    if(readLen == -1) {
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    fd = open(fileName, O_CREAT | O_WRONLY);
    if(fd == -1) {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }

    write(fd, msg.mtext, readLen);
    close(fd);

    if(msgctl(queueId, IPC_RMID, NULL) == -1) {
        perror("msgctl failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
