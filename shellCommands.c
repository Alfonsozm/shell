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
        fprintf(stdout,"[%d] \t%7s \t%s",p->id,"Running",p->line); //TODO
        n = n->next;
    }
}

void foreground(processHandler_t *processHandler, int i) {
    process_t const *p = NULL;
    node_t *n = processHandler->background->first;
    while (n != NULL && p ==NULL) {
        if (((process_t *) n->info)->id == i) {
            p = (process_t *) n->info;
        }
        n = n->next;
    }

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d id", i);
    } else {
        for (int j = 0; j < p->count; ++j) {
            kill(p->pid[j], SIGUSR1);
        }
        for (int j = 0; j < p->count; ++j) {
            kill(p->pid[j], SIGCONT);
        }
    }
}

void background(processHandler_t *processHandler, int i) {
    process_t const *p = NULL;
    node_t *n = processHandler->background->first;
    while (n != NULL) {
        if (((process_t *) n->info)->id == i) {
            p = (process_t *) n->info;
            break;
        }
        n = n->next;
    }

    if (p == NULL) {
        fprintf(stderr, "There is no job with %d id", i);
    } else {
        //TODO
    }
}