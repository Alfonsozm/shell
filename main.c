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

int stdinBACKUP;
int stdoutBACKUP;
int stderrBACKUP;
int devNULL;
processHandler_t *processHandler;

//if any stream is -1 no redirection is made, if inputStream is -2 stdin is closed
pid_t createNewProcess(tcommand command, int inputStream, int outputStream, int errorStream);

void sigtstpHandler(int sig);

void sigusr1Handler(int sig);

int main(void) {
    stdinBACKUP = dup(fileno(stdin));
    stdoutBACKUP = dup(fileno(stdout));
    stderrBACKUP = dup(fileno(stderr));
    devNULL = open("/dev/null", O_WRONLY);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
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
            printf("msh (%s) > ", cwd);
            continue;
        }

        int inputStream = -1;
        int outputStream = -1;
        int errorStream = -1;
        pid_t *pid = malloc(sizeof(pid_t) * line->ncommands);
        if (line->redirect_input != NULL) {
            inputStream = open(line->redirect_input, O_RDONLY);
        }
        if (line->redirect_output != NULL) {
            outputStream = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC,
                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        }
        if (line->redirect_error != NULL) {
            errorStream = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        }
        if (line->ncommands == 1) {
            if (line->commands[0].filename == NULL) {
                if (!strcmp(line->commands[0].argv[0], "exit")) {
                    if (line->commands[0].argc == 1) {
                        node_t *n = processHandler->background->first;
                        while (n != NULL) {
                            process_t const *p = (process_t *) n->info;
                            for (int i = 0; i < p->count; ++i) {
                                kill(p->pid[i], SIGTERM); //TODO check whether a process has already died
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
                    if (line->commands[0].argc == 2) {
                        foreground(processHandler, (int) strtol(line->commands[0].argv[1], NULL, 10));
                    } else {
                        fprintf(stderr, "Incorrect number of arguments for command \"%s\"\n",
                                line->commands[0].argv[0]);
                    }
                } else if (!strcmp(line->commands[0].argv[0], "bg") ||
                           !strcmp(line->commands[0].argv[0], "background")) {
                    if (line->commands[0].argc == 2) {
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
                sleep(1);
            } else if (line->background) {
                pid[0] = createNewProcess(line->commands[0], inputStream == -1 ? -2 : inputStream,
                                          outputStream == -1 ? devNULL : outputStream,
                                          errorStream == -1 ? devNULL : errorStream);
            } else {
                pid[0] = createNewProcess(line->commands[0], inputStream,
                                          outputStream,
                                          errorStream);
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
                if (line->background) {
                    pipe(pipeline);
                    pid[0] = createNewProcess(line->commands[0], inputStream == -1 ? -2 : inputStream,
                                              pipeline[1],
                                              errorStream == -1 ? devNULL : errorStream);
                    close(pipeline[1]);
                    int aux;
                    for (int i = 1; i < line->ncommands - 1; ++i) {
                        aux = pipeline[0];
                        pipe(pipeline);
                        pid[i] = createNewProcess(line->commands[i], aux, pipeline[1],
                                                  errorStream == -1 ? devNULL : errorStream);
                        close(aux);
                        close(pipeline[1]);
                    }
                    pid[line->ncommands - 1] = createNewProcess(line->commands[line->ncommands - 1], pipeline[0],
                                                                outputStream == -1 ? devNULL : outputStream,
                                                                errorStream == -1 ? devNULL : errorStream);
                    close(pipeline[0]);
                } else {
                    pipe(pipeline);
                    pid[0] = createNewProcess(line->commands[0], inputStream,
                                              pipeline[1],
                                              errorStream);
                    close(pipeline[1]);
                    int aux;
                    for (int i = 1; i < line->ncommands - 1; ++i) {
                        aux = pipeline[0];
                        pipe(pipeline);
                        pid[i] = createNewProcess(line->commands[i], aux, pipeline[1],
                                                  errorStream);
                        close(aux);
                        close(pipeline[1]);
                    }
                    pid[line->ncommands - 1] = createNewProcess(line->commands[line->ncommands - 1], pipeline[0],
                                                                outputStream,
                                                                errorStream);
                    close(pipeline[0]);
                }
            }
        }

        if (!check) {
            process_t *process = malloc(sizeof(process_t));
            process->line = buf;
            process->id = -1;
            process->count = line->ncommands;
            process->pid = pid;
            if (line->background) {
                addBackground(processHandler, process);
            } else {
                addForeground(processHandler, process);
                for (int i = 0; i < line->ncommands; ++i) {
                    waitpid(pid[i], NULL, 0);
                }
                removeForeground(processHandler);
            }

            buf = malloc(sizeof(char) * 1024);
            getcwd(cwd, 1024);
        } else {
            free(pid);
            if (buf != NULL) free(buf);
            buf = malloc(sizeof(char) * 1024);
        }
        printf("msh (%s) > ", cwd);
    }
    cleanProcessHandler(processHandler);
    free(buf);
    free(cwd);
    return 0;
}

pid_t createNewProcess(tcommand command, int inputStream, int outputStream, int errorStream) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error while forking\n");
        exit(1);
    } else if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        if (inputStream != -1) {
            if (inputStream == -2) {
                close(fileno(stdin));
            } else {
                dup2(inputStream, fileno(stdin));
            }
        }
        if (outputStream != -1) {
            dup2(outputStream, fileno(stdout));
        }
        if (errorStream != -1) {
            dup2(errorStream, fileno(stderr));
        }
        execvp(command.filename, command.argv);
    }
    return pid;
}

void sigtstpHandler(int sig) {
    process_t *p = getForeground(processHandler);
    for (int i = 0; i < p->count; ++i) {
        kill(p->pid[i], SIGSTOP);
    }
    processHandler->foreground = NULL;
    addBackground(processHandler, p);
}

void sigusr1Handler(int sig) {
    //TODO
}