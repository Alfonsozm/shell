#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include "parser.h"
#include "processHandler.h"

int main(void) {
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
        if (line->redirect_input != NULL) {
            //TODO
            printf("redirección de entrada: %s\n", line->redirect_input);
        }
        if (line->redirect_output != NULL) {
            //TODO
            printf("redirección de salida: %s\n", line->redirect_output);
        }
        if (line->redirect_error != NULL) {
            //TODO
            printf("redirección de error: %s\n", line->redirect_error);
        }
        if (line->background) {
            //TODO
            printf("comando a ejecutarse en background\n");
        }
        pid_t *pid = malloc(sizeof(pid_t) * line->ncommands);
        for (int i = 0; i < line->ncommands; i++) {
            pid[i] = fork();
            if (pid[i] < 0) {
                perror("Error while forking");
                exit(1);
            } else if (pid[i] == 0) {
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                execvp(line->commands[i].filename, line->commands[i].argv);
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
            int aux = 1;
            while(aux) {
                aux = 0;
                for (int i = 0; i < line->ncommands; ++i) {
                    aux = aux || !kill(pid[i],0);
                }
            }
            removeForeground(processHandler);
        }

        buf = malloc(sizeof(char) * 1024);
        getcwd(cwd, 1024);
        printf("msh (%s) > ", cwd);
    }
    return 0;
}

