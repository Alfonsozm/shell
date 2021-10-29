#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include "parser.h"
#include "processHandler.h"

//TODO exit, cd, jobs, fg, foreground

//if any stream is -1 no redirection is made, if inputStream is -2 stdin is closed
pid_t createNewProcess(tcommand command, int inputStream, int outputStream, int errorStream);

int main(void) {
    //int stdinBACKUP = dup(fileno(stdin));
    //int stdoutBACKUP = dup(fileno(stdout));
    //int stderrBACKUP = dup(fileno(stderr));
    int devNULL = open("/dev/null", O_WRONLY);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    char *cwd = malloc(sizeof(char) * 1024);
    char *buf = malloc(sizeof(char) * 1024);
    processHandler_t *processHandler = createEmptyProcessHandler();

    getcwd(cwd, 1024);
    printf("msh (%s) > ", cwd);
    while (fgets(buf, 1024, stdin)) {

        tline *line = tokenize(buf);
        if (line == NULL) {
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
            if (line->background) {
                pid[0] = createNewProcess(line->commands[0], inputStream == -1 ? -2 : inputStream,
                                          outputStream == -1 ? devNULL : outputStream,
                                          errorStream == -1 ? devNULL : errorStream);
            } else {
                pid[0] = createNewProcess(line->commands[0], inputStream,
                                          outputStream,
                                          errorStream);
            }
        } else {
            if (line->background) {
                int pipeline[2];
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
                int pipeline[2];
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


        process_t *process = malloc(sizeof(process_t));
        process->line = buf;
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
        printf("msh (%s) > ", cwd);
    }
    free(buf);
    free(cwd);
    return 0;
}

pid_t createNewProcess(tcommand command, int inputStream, int outputStream, int errorStream) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error while forking");
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

