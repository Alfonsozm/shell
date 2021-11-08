#include "processHandler.h"

processHandler_t *createEmptyProcessHandler() {
    processHandler_t *processHandler = (processHandler_t *) malloc(sizeof(processHandler_t));
    processHandler->foreground = NULL;
    processHandler->totalBackgroundProcessesAdded = 0;
    processHandler->background = createEmptyList();
    return processHandler;
}

process_t *createNewProcess(char *line, int id, int count, pid_t *pid) {
    process_t *p = (process_t *) malloc(sizeof(process_t));
    p->line = line;
    p->jobId = id;
    p->count = count;
    p->pid = pid;
    p->groupPid = pid[0];
    p->groupStatus = RUNNING;
    p->pidStatus = malloc(sizeof(status_t) * count);
    for (int i = 0; i < count; ++i) {
        p->pidStatus[i] = RUNNING;
    }
    return p;
}

void checkProcessStatus(process_t *p) {
    if (p->groupStatus != ENDED) {
        for (int i = 0; i < p->count; ++i) {
            if (p->pidStatus[i] != ENDED && waitpid(p->pid[i], NULL, WNOHANG) > 0) {
                p->pidStatus[i] = ENDED;
            }
        }

        int check = 1;
        for (int i = 0; i < p->count; ++i) {
            check = check && p->pidStatus[i] == ENDED;
        }
        if (check) {
            p->groupStatus = ENDED;
        }
    }
}

void cleanProcess(process_t *process) {
    free(process->line);
    free(process->pid);
    free(process->pidStatus);
    free(process);
}

void cleanProcessHandler(processHandler_t *processHandler) {
    node_t *n = processHandler->background->first;
    while (n != NULL) {
        free(n->previous);
        cleanProcess((process_t *) n->info);
        n = n->next;
    }
    free(processHandler->background);
    removeForeground(processHandler);
    free(processHandler);
}

void addForeground(processHandler_t *processHandler, process_t *process) {
    processHandler->foreground = process;
}

process_t *getForeground(processHandler_t const *processHandler) {
    return processHandler->foreground;
}

void removeForeground(processHandler_t *processHandler) {
    if (processHandler->foreground != NULL) {
        cleanProcess(processHandler->foreground);
        processHandler->foreground = NULL;
    }
}

int addBackground(processHandler_t *processHandler, process_t *process) {
    addInfo(processHandler->background, process);
    if (process->jobId == -1) {
        processHandler->totalBackgroundProcessesAdded++;
        process->jobId = processHandler->totalBackgroundProcessesAdded;
    }
    return processHandler->totalBackgroundProcessesAdded;
}

process_t *removeBackground(processHandler_t *processHandler, int jobId) {
    node_t *n = processHandler->background->first;
    process_t *p = NULL;
    if(n!=NULL && ((process_t*)n->info)->jobId == jobId){
        p = (process_t*) n->info;
    }
    while (n != NULL && p == NULL){
        n = n->next;
        if(((process_t*)n->info)->jobId == jobId){
            p = (process_t*) n->info;
        }
    }

    if(p!=NULL){
        node_t *previous = n->previous;
        node_t *next = n->next;
        if(previous != NULL){
            previous->next = next;
        } else {
            processHandler->background->first = next;
        }
        if(next != NULL){
            next->previous = previous;
        } else {
            processHandler->background->last = previous;
        }
        processHandler->background->count--;
    }

    return p;
}

list_t *getBackground(processHandler_t const *processHandler) {
    return processHandler->background;
}