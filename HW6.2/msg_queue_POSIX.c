#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HANDLE_CODE( FUNC, MSG, CODE, ... ) \
    do { \
        if((FUNC) == CODE) { \
            perror(MSG); \
            __VA_ARGS__; \
            return EXIT_FAILURE; \
        } \
    } while(0)

#define HANDLE_ERROR( FUNC, MSG, ... ) \
    HANDLE_CODE( FUNC, MSG, -1, __VA_ARGS__ )

#define HANDLE_NULL( FUNC, MSG, ... ) \
    HANDLE_CODE( FUNC, MSG, NULL, __VA_ARGS__ )

int main() {
    mqd_t mqd;
    struct mq_attr attr;
    void* buffer;
    ssize_t bytesRead;
    int fd;
    const char outFileName[] = "/home/box/message.txt";
    const char msgQueueName[] = "/test.mq";

    HANDLE_ERROR(
            mqd = mq_open(msgQueueName, O_CREAT | O_RDONLY),
            "mq_open", goto unlink );

    HANDLE_ERROR(
            mq_getattr(mqd, &attr),
            "mq_getattr", goto unlink );

    HANDLE_NULL(
            buffer = malloc(attr.mq_msgsize),
            "malloc", goto unlink );

    HANDLE_ERROR(
            bytesRead = mq_receive(mqd, buffer, attr.mq_msgsize, NULL),
            "mq_receive", goto unlink );

    HANDLE_ERROR(
            fd = open(outFileName, O_CREAT | O_WRONLY),
            "open", goto close );

    HANDLE_ERROR(
            write(fd, buffer, bytesRead),
            "write" );

close:
    HANDLE_ERROR( close(fd), "close" );
unlink:
    HANDLE_ERROR( mq_unlink(msgQueueName), "mq_unlink" );

    exit(EXIT_SUCCESS);
}
