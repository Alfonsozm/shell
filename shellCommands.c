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

void foreground(processHandler_t *processHandler, int i) {
    process_t *p = NULL;
    node_t *n = processHandler->background->first;
    while (n != NULL && p == NULL) {
        if (((process_t *) n->info)->jobId == i) {
            p = (process_t *) n->info;
        }
        n = n->next;
    }

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d jobId", i);
    } else if (p->groupStatus == ENDED) {
        fprintf(stderr, "The job with jobId %d has already ended", i);
    } else {
        //killpg(p->groupPid, SIGUSR1);
        killpg(p->groupPid, SIGCONT);
        for (int j = 0; j < p->count; ++j) {
            if (p->pidStatus[j] != ENDED) {
                p->pidStatus[j] = RUNNING;
            }
        }
        p->groupStatus = RUNNING;
        removeBackground(processHandler,p->jobId);
        addForeground(processHandler,p);
    }
}

void background(processHandler_t *processHandler, int i) {
    process_t const *p = NULL;
    node_t *n = processHandler->background->first;
    while (n != NULL) {
        if (((process_t *) n->info)->jobId == i) {
            p = (process_t *) n->info;
            break;
        }
        n = n->next;
    }

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d jobId", i);
    } else {
        //TODO
    }
}