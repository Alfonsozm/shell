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
    signal(SIGSTOP, SIG_IGN);
    char *buf = malloc(sizeof(char) * 1024);
    processHandler_t *processHandler = createEmptyProcessHandler();

    printf("msh > ");
    while (fgets(buf, 1024, stdin)) {

        tline *line = tokenize(buf);
        if (line == NULL) {
            continue;
        }
        if (line->redirect_input != NULL) {
            printf("redirección de entrada: %s\n", line->redirect_input);
        }
        if (line->redirect_output != NULL) {
            printf("redirección de salida: %s\n", line->redirect_output);
        }
        if (line->redirect_error != NULL) {
            printf("redirección de error: %s\n", line->redirect_error);
        }
        if (line->background) {
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
                signal(SIGSTOP, SIG_DFL);
                execvp(line->commands[i].filename, line->commands[i].argv);
            }
        }
        process_t *process = malloc(sizeof(process_t));
        process->line = buf;
        process->count = line->ncommands;
        process->pid = pid;
        if (line->background) {

        } else {
            addForeground(processHandler, process);
            for (int i = 0; i < line->ncommands; ++i) {
                waitpid(pid[i], NULL, 0);
            }
            removeForeground(processHandler);
        }

        buf = malloc(sizeof(char) * 1024);
        printf("msh > ");
    }
    return 0;
}

