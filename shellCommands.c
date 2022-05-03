#include "shellCommands.h"

void shellCommand(processHandler_t *processHandler, tcommand *command, char* cwd){
    if (!strcmp(command->argv[0], "exit")) {
        if (command->argc == 1) {
            node_t *n = getBackground(processHandler)->first;
            while (n != NULL) {
                process_t const *p = (process_t *) n->info;
                if (p->groupStatus != ENDED) {
                    killpg(p->groupPid, SIGTERM);
                    for (int i = 0; i < 3; ++i) {
                        waitpid(p->ioHandlers[i],NULL, 0);
                    }
                }
                n = n->next;
            }
            cleanProcessHandler_T(processHandler);
            exit(0);
        } else {
            fprintf(stderr, "Incorrect number of arguments for command \"exit\"\n");
        }
    } else if (!strcmp(command->argv[0], "cd")) {
        if (command->argc <= 2) {
            cd(command->argv[1]);
            getcwd(cwd, 1024);
        } else {
            fprintf(stderr, "Incorrect number of arguments for command \"cd\"\n");
        }
    } else if (!strcmp(command->argv[0], "fg") ||
               !strcmp(command->argv[0], "foreground")) {
        if (command->argc == 1) {
            foreground(processHandler, 0);
        } else if (command->argc == 2) {
            foreground(processHandler, (int) strtol(command->argv[1], NULL, 10));
        } else {
            fprintf(stderr, "Incorrect number of arguments for command \"%s\"\n",
                    command->argv[0]);
        }
    } else if (!strcmp(command->argv[0], "bg") ||
               !strcmp(command->argv[0], "background")) {
        if (command->argc == 1) {
            background(processHandler, 0);
        } else if (command->argc == 2) {
            background(processHandler, (int) strtol(command->argv[1], NULL, 10));
        } else {
            fprintf(stderr, "Incorrect number of arguments for command \"%s\"\n",
                    command->argv[0]);
        }
    } else if (!strcmp(command->argv[0], "jobs")) {
        if (command->argc == 1) {
            jobs(processHandler);
        } else {
            fprintf(stderr, "Incorrect number of arguments for command \"jobs\"\n");
        }
    } else {
        fprintf(stderr, "Command \"%s\" does not exist\n", command->argv[0]);
    }
}

int cd(char const *dir) {
    int check;
    if (dir == NULL) {
        check = chdir(getenv("HOME"));
    } else {
        check = chdir(dir);
    }

    if (check == 0) {
        return 0;
    } else {
        fprintf(stderr, "Incorrect directory name\n");
        return -1;
    }
}

void jobs(processHandler_t const *processHandler) {
    fprintf(stdout, "Id     \tStatus  \tProgram\n");
    node_t *n = processHandler->background->first;

    while (n != NULL) {
        process_t *p = (process_t *) n->info;
        checkProcessStatus(p);
        char *c;
        if (p->groupStatus == RUNNING) {
            c = "Running";
        } else if (p->groupStatus == STOPPED) {
            c = "Stopped";
        } else {
            c = "Ended";
        }
        fprintf(stdout, "[%d] \t%7s \t%s", p->jobId, c, p->line);
        n = n->next;
    }
}

void foreground(processHandler_t *processHandler, int jobId) {
    if(jobId == 0){
        process_t const *aux = processHandler->background->last->info;
        jobId = aux->jobId;
    }
    process_t *p = removeBackground(processHandler, jobId);

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d jobId", jobId);
    } else if (p->groupStatus == ENDED) {
        fprintf(stderr, "The job with jobId %d has already ended", jobId);
    } else {
        killpg(p->groupPid, SIGCONT);
        for (int i = 0; i < 3; ++i) {
            if (!p->hasRedirection[i]) {
                kill(p->ioHandlers[i], SIGUSR1);
            }
        }
        for (int j = 0; j < p->count; ++j) {
            if (p->pidStatus[j] != ENDED) {
                p->pidStatus[j] = RUNNING;
            }
        }
        p->groupStatus = RUNNING;
        setForeground(processHandler, p);
        while (p->groupStatus == RUNNING) {
            checkProcessStatus(p);
        }
        if (p->groupStatus == ENDED) {
            sleep(1);
            killpg(p->groupPid, SIGTERM);
        }
        removeForeground(processHandler);
    }
}

void background(processHandler_t *processHandler, int jobId) {
    if(jobId == 0){
        process_t const *aux = processHandler->background->last->info;
        jobId = aux->jobId;
    }
    process_t *p = removeBackground(processHandler, jobId);
    addBackground(processHandler,p);

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d jobId", jobId);
    } else if (p->groupStatus == ENDED) {
        fprintf(stderr, "The job with jobId %d has already ended", jobId);
    } else {
        killpg(p->groupPid, SIGCONT);
        for (int i = 0; i < 3; ++i) {
            if (!p->hasRedirection[i]) {
                kill(p->ioHandlers[i], SIGUSR2);
            }
        }
        killpg(p->groupPid, SIGCONT);
        for (int j = 0; j < p->count; ++j) {
            if (p->pidStatus[j] != ENDED) {
                p->pidStatus[j] = RUNNING;
            }
        }
        p->groupStatus = RUNNING;
    }
}