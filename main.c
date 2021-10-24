#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include "parser.h"
#include "processHandler.h"
#include "handler.h"

int main(void) {
    signal(SIGINT, handleSIGINT);
    signal(SIGQUIT, handleSIGQUIT);
    signal(SIGUSR1, handleSIGUSR1);
    char *buf = malloc(sizeof(char) * 1024);

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
                pause();
                execvp(line->commands[i].filename, line->commands[i].argv);
            }
        }
        process_t process;
        process.line = buf;
        process.count = line->ncommands;
        process.pid = pid;
        if (line->background) {

        } else {
            addForeground(process);
            sleep(1); //thread synchronization
            for (int i = 0; i < line->ncommands; ++i) {
                kill(getForegroundPid()[i], SIGUSR1);
            }

            for (int i = 0; i < line->ncommands; ++i) {
                waitpid(pid[i], NULL, 0);
            }
            removeForeground();
        }

        buf = malloc(sizeof(char) * 1024);
        printf("msh > ");
    }
    return 0;
}

