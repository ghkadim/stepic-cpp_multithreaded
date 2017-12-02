#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 512

int main(int argc, char* argv[]) {
    const char outFileName[] = "/home/box/result.out";
    char input[MAX_INPUT_SIZE];
    char cmd[MAX_INPUT_SIZE];

    FILE* cmdFile;
    FILE* outFile;

    if(fgets(cmd, sizeof(cmd), stdin) == NULL) {
        fprintf(stderr, "Invalid input from stdin\n");
        exit(EXIT_FAILURE);
    }

    cmdFile = popen(cmd, "r");
    if(cmdFile == NULL) {
        fprintf(stderr, "popen() failed\n");
        exit(EXIT_FAILURE);
    }

    outFile = fopen(outFileName, "w");
    if(outFile == NULL) {
        fprintf(stderr, "fopen() failed\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(input, sizeof(input), cmdFile) != NULL) {
        fprintf(outFile, input);
    }

    fclose(outFile);
    pclose(cmdFile);
    exit(EXIT_SUCCESS);
}
