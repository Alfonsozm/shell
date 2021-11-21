#include "ioHandler.h"

void init() {
    stdinBACKUP = dup(STDIN_FILENO);
    stdoutBACKUP = dup(STDOUT_FILENO);
    stderrBACKUP = dup(STDERR_FILENO);
    devNULL = open("/dev/null", O_WRONLY);
}

pid_t createNewIOHandler(int pipeIn, int pipeOut, io_t io) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error while forking\n");
        exit(1);
    } else if (pid == 0) {
        status = ON;
        ioType = io;
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGUSR1, sigusr1Handler);
        signal(SIGUSR2, sigusr2Handler);
        dup2(pipeIn, STDIN_FILENO);
        dup2(pipeOut, STDOUT_FILENO);
        char *buffer = (char *) malloc(sizeof(char) * 1024);
        while (1) {
            if (status == ON) {
                fgets(buffer, 1024, stdin);
                fputs(buffer, stdout);
            }
        }
    }
    fprintf(stderr, "IOHandler %d %d\n", io, pid);
    return pid;
}

pid_t createNewIOHandlerOFF(int pipeIn, int pipeOut, io_t io) {
    if (ioType != IN) {
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error while forking\n");
        exit(1);
    } else if (pid == 0) {
        status = OFF;
        ioType = io;
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGUSR1, sigusr1Handler);
        signal(SIGUSR2, sigusr2Handler);
        dup2(pipeIn, STDIN_FILENO);
        dup2(pipeOut, STDOUT_FILENO);
        char *buffer = (char *) malloc(sizeof(char) * 1024);
        while (1) {
            if (status == ON) {
                fgets(buffer, 1024, stdin);
                fputs(buffer, stdout);
            }
        }
    }
    fprintf(stderr, "IOHandler %d %d\n", io, pid);
    return pid;
}

void sigusr1Handler(int sig) {
    switch (ioType) {
        case IN:
            status = ON;
            break;
        case OUT:
            dup2(stdoutBACKUP, STDOUT_FILENO);
            break;
        case ERR:
            dup2(stderrBACKUP, STDOUT_FILENO);
            break;
        default:
            break;
    }
}

void sigusr2Handler(int sig) {
    switch (ioType) {
        case IN:
            status = OFF;
            break;
        case OUT:
        case ERR:
            dup2(devNULL, STDOUT_FILENO);
            break;
        default:
            break;
    }
}