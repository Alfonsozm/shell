#include "shellCommands.h"

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
        fprintf(stdout, "[%d] \t%7s \t%s\n", p->jobId, c, p->line);
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
        addForeground(processHandler, p);
        while (p->groupStatus == RUNNING) {
            checkProcessStatus(p);
        }
        if (p->groupStatus == ENDED) {
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
        for (int j = 0; j < p->count; ++j) {
            if (p->pidStatus[j] != ENDED) {
                p->pidStatus[j] = RUNNING;
            }
        }
        p->groupStatus = RUNNING;
    }
}