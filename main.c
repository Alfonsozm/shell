#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "parser.h"
#include "processHandler.h"
#include "shellCommands.h"
#include "ioHandler.h"

processHandler_t *processHandler;

//if any stream is -2 no redirection is made, if inputStream is -3 stdin is closed
pid_t createNewProcessLine(tcommand command, int inputStream, int outputStream, int errorStream, pid_t groupPid);

void sigtstpHandler(int sig);

void foregroundSignalHandler(int sig);

void sigtermHandler(int sig);

int main(void) {
    init();
    signal(SIGTERM, sigtermHandler);
    signal(SIGINT, foregroundSignalHandler);
    signal(SIGQUIT, foregroundSignalHandler);
    signal(SIGTSTP, sigtstpHandler);
    char *cwd = malloc(sizeof(char) * 1024);
    char *buf = malloc(sizeof(char) * 1024);
    processHandler = createEmptyProcessHandler();

    getcwd(cwd, 1024);
    printf("msh (%s) > ", cwd);
    while (fgets(buf, 1024, stdin)) {
        int check = 0; //whether a command is executed or not (if false a command is executed)
        tline const *line = tokenize(buf);
        if (line == NULL || line->ncommands <= 0) {
            fflush(stderr);
            printf("msh (%s) > ", cwd);
            continue;
        }
        int inputStream = -2;
        int outputStream = -2;
        int errorStream = -2;
        int *hasRedirection = malloc(sizeof(int) * 3);
        for (int i = 0; i < 3; ++i) {
            hasRedirection[i] = 0;
        }
        if (line->redirect_input != NULL) {
            hasRedirection[0] = 1;
            inputStream = open(line->redirect_input, O_RDONLY);
        }
        if (line->redirect_output != NULL) {
            hasRedirection[1] = 1;
            outputStream = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        }
        if (line->redirect_error != NULL) {
            hasRedirection[2] = 1;
            errorStream = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

        }
        if (inputStream == -1 || outputStream == -1 || errorStream == -1) {
            free(hasRedirection);
            fprintf(stderr,
                    "There has been a problem when opening the files to redirect input or output, check the filenames and try again\n");
            fflush(stderr);
            printf("msh (%s) > ", cwd);
            continue;
        }
        pid_t *pid = (pid_t *) malloc(sizeof(pid_t) * line->ncommands);
        pid_t *ioHandler = (pid_t *) malloc(sizeof(pid_t) * 3);
        int **ioPipe = (int **) malloc(sizeof(int *) * 3);
        for (int i = 0; i < 3; ++i) {
            ioPipe[i] = malloc(sizeof(int) * 2);
        }
        if (line->ncommands == 1) {
            if (line->commands[0].filename == NULL) {
                check = 1;
                if (!strcmp(line->commands[0].argv[0], "exit")) {
                    if (line->commands[0].argc == 1) {
                        node_t *n = getBackground(processHandler)->first;
                        while (n != NULL) {
                            process_t const *p = (process_t *) n->info;
                            if (p->groupStatus != ENDED) {
                                killpg(p->groupPid, SIGTERM);
                            }
                            n = n->next;
                        }
                        cleanProcessHandler(processHandler);
                        exit(0);
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"exit\"\n");
                    }
                } else if (!strcmp(line->commands[0].argv[0], "cd")) {
                    if (line->commands[0].argc <= 2) {
                        cd(line->commands[0].argv[1]);
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"cd\"\n");
                    }
                } else if (!strcmp(line->commands[0].argv[0], "fg") ||
                           !strcmp(line->commands[0].argv[0], "foreground")) {
                    if (line->commands[0].argc == 1) {
                        foreground(processHandler, 0);
                    }else if (line->commands[0].argc == 2) {
                        foreground(processHandler, (int) strtol(line->commands[0].argv[1], NULL, 10));
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"%s\"\n",
                                line->commands[0].argv[0]);
                    }
                } else if (!strcmp(line->commands[0].argv[0], "bg") ||
                           !strcmp(line->commands[0].argv[0], "background")) {
                    if (line->commands[0].argc == 1) {
                        background(processHandler, 0);
                    }else if (line->commands[0].argc == 2) {
                        background(processHandler, (int) strtol(line->commands[0].argv[1], NULL, 10));
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"%s\"\n",
                                line->commands[0].argv[0]);
                    }
                } else if (!strcmp(line->commands[0].argv[0], "jobs")) {
                    if (line->commands[0].argc == 1) {
                        jobs(processHandler);
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"jobs\"\n");
                    }
                } else {
                    fprintf(stderr, "Command \"%s\" does not exist\n", line->commands[0].argv[0]);
                }
            } else if (line->background) {
                for (int i = 0; i < 3; ++i) {
                    while (pipe(ioPipe[i]) == -1);
                }
                if (inputStream == -2) {
                    ioHandler[0] = createNewIOHandlerOFF(STDIN_FILENO, ioPipe[0][1], IN);
                } else {
                    ioHandler[0] = createNewIOHandler(inputStream, ioPipe[0][1], IN);
                }
                ioHandler[1] = createNewIOHandler(ioPipe[1][0], outputStream == -2 ? devNULL : outputStream, OUT);
                ioHandler[2] = createNewIOHandler(ioPipe[2][0], errorStream == -2 ? devNULL : errorStream, ERR);
                pid[0] = createNewProcessLine(line->commands[0], ioPipe[0][0],
                                              ioPipe[1][1],
                                              ioPipe[2][1], 0);
                for (int i = 0; i < 3; ++i) {
                    while (setpgid(ioHandler[i], pid[0]) == -1);
                }
            } else {
                for (int i = 0; i < 3; ++i) {
                    while (pipe(ioPipe[i]) == -1);
                }
                ioHandler[0] = createNewIOHandler(inputStream == -2 ? STDIN_FILENO : inputStream, ioPipe[0][1], IN);
                ioHandler[1] = createNewIOHandler(ioPipe[1][0], outputStream == -2 ? STDOUT_FILENO : outputStream, OUT);
                ioHandler[2] = createNewIOHandler(ioPipe[2][0], errorStream == -2 ? STDERR_FILENO : errorStream, ERR);
                pid[0] = createNewProcessLine(line->commands[0], ioPipe[0][0],
                                              ioPipe[1][1],
                                              ioPipe[2][1], 0);
                for (int i = 0; i < 3; ++i) {
                    while (setpgid(ioHandler[i], pid[0]) == -1);
                }
            }
        } else {
            for (int i = 0; i < line->ncommands; ++i) {
                check = check || line->commands[i].filename == NULL;
            }
            if (check) {
                fprintf(stderr, "Incorrect command input\n"
                                "Either command one or more commands do not exist or any of the commands \"exit\", \"cd\", \"jobs\", \"fg\", \"foreground\", \"bg\" or \"background\" are used\n");
            } else {
                int pipeline[2];
                for (int i = 0; i < 3; ++i) {
                    while (pipe(ioPipe[i]));
                }
                if (line->background) {
                    if (inputStream == -2) {
                        ioHandler[0] = createNewIOHandlerOFF(STDIN_FILENO, ioPipe[0][1], IN);
                    } else {
                        ioHandler[0] = createNewIOHandler(inputStream, ioPipe[0][1], IN);
                    }
                    ioHandler[1] = createNewIOHandler(ioPipe[1][0], outputStream == -2 ? devNULL : outputStream, OUT);
                    ioHandler[2] = createNewIOHandler(ioPipe[2][0], errorStream == -2 ? devNULL : errorStream, ERR);
                    while (pipe(pipeline) == -1);
                    pid[0] = createNewProcessLine(line->commands[0], ioPipe[0][0],
                                                  pipeline[1],
                                                  ioPipe[2][1], 0);
                    close(pipeline[1]);
                    int aux;
                    for (int i = 1; i < line->ncommands - 1; ++i) {
                        aux = pipeline[0];
                        while (pipe(pipeline) == -1);
                        pid[i] = createNewProcessLine(line->commands[i], aux, pipeline[1],
                                                      ioPipe[2][1], pid[0]);
                        close(aux);
                        close(pipeline[1]);
                    }
                    pid[line->ncommands - 1] = createNewProcessLine(line->commands[line->ncommands - 1], pipeline[0],
                                                                    ioPipe[1][1],
                                                                    ioPipe[2][1], pid[0]);
                    close(pipeline[0]);
                } else {
                    ioHandler[0] = createNewIOHandler(inputStream == -2 ? STDIN_FILENO : inputStream, ioPipe[0][1], IN);
                    ioHandler[1] = createNewIOHandler(ioPipe[1][0], outputStream == -2 ? STDOUT_FILENO : outputStream, OUT);
                    ioHandler[2] = createNewIOHandler(ioPipe[2][0], errorStream == -2 ? STDERR_FILENO : errorStream, ERR);
                    while (pipe(pipeline) == -1);
                    pid[0] = createNewProcessLine(line->commands[0], ioPipe[0][0],
                                                  pipeline[1],
                                                  ioPipe[2][1], 0);
                    close(pipeline[1]);
                    int aux;
                    for (int i = 1; i < line->ncommands - 1; ++i) {
                        aux = pipeline[0];
                        while (pipe(pipeline) == -1);
                        pid[i] = createNewProcessLine(line->commands[i], aux, pipeline[1],
                                                      ioPipe[2][1], pid[0]);
                        close(aux);
                        close(pipeline[1]);
                    }
                    pid[line->ncommands - 1] = createNewProcessLine(line->commands[line->ncommands - 1], pipeline[0],
                                                                    ioPipe[1][1],
                                                                    ioPipe[2][1], pid[0]);
                    close(pipeline[0]);
                }
                for (int i = 0; i < 3; ++i) {
                    while (setpgid(ioHandler[i], pid[0]) == -1);
                }
            }
        }

        if (!check) {
            process_t *process = createNewProcess(buf, -1, line->ncommands, pid, ioHandler, hasRedirection);
            if (line->background) {
                addBackground(processHandler, process);
            } else {
                addForeground(processHandler, process);
                while (process->groupStatus == RUNNING) {
                    checkProcessStatus(process);
                }
                if (process->groupStatus == ENDED) {
                    killpg(process->groupPid, SIGTERM);
                }
                removeForeground(processHandler);
            }

            buf = malloc(sizeof(char) * 1024);
            getcwd(cwd, 1024);
        } else {
            free(pid);
            free(ioHandler);
            free(hasRedirection);
            if (buf != NULL) free(buf);
            buf = malloc(sizeof(char) * 1024);
        }
        for (int i = 0; i < 3; ++i) {
            free(ioPipe[i]);
        }
        free(ioPipe);
        fflush(stderr);
        fflush(stdout);
        printf("msh (%s) > ", cwd);
    }
    cleanProcessHandler(processHandler);
    free(buf);
    free(cwd);
    return 0;
}

pid_t createNewProcessLine(tcommand command, int inputStream, int outputStream, int errorStream, pid_t groupPid) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error while forking\n");
        exit(1);
    } else if (pid == 0) {
        setpgid(0, groupPid);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        dup2(inputStream, STDIN_FILENO);
        dup2(outputStream, STDOUT_FILENO);
        dup2(errorStream, STDERR_FILENO);

        execvp(command.filename, command.argv);
        exit(-1);
    }
    return pid;
}

void sigtstpHandler(int sig) {
    process_t *p = getForeground(processHandler);
    if (p != NULL) {
        killpg(p->groupPid, SIGSTOP);
        processHandler->foreground = NULL;
        p->groupStatus = STOPPED;
        int i = addBackground(processHandler, p);
        fprintf(stdout, "Sent to background with job jobId: %d\n", i);
    }
}

void foregroundSignalHandler(int sig) {
    process_t const *p = getForeground(processHandler);
    if (p != NULL) {
        killpg(p->groupPid, sig);
    }
}

void sigtermHandler(int sig) {
    if(processHandler->foreground!=NULL) {
        killpg(processHandler->foreground->groupPid, SIGTERM);
    }
    node_t *n = getBackground(processHandler)->first;
    while (n != NULL) {
        process_t const *p = (process_t *) n->info;
        if (p->groupStatus != ENDED) {
            killpg(p->groupPid, SIGTERM);
        }
        n = n->next;
    }
    cleanProcessHandler(processHandler);
    exit(0);
}