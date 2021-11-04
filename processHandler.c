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
    p->id = id;
    p->count = count;
    p->pid = pid;
    return p;
}

void cleanProcess(process_t *process) {
    free(process->line);
    free(process->pid);
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
    if (process->id == -1) {
        processHandler->totalBackgroundProcessesAdded++;
        process->id = processHandler->totalBackgroundProcessesAdded;
    }
    return processHandler->totalBackgroundProcessesAdded;
}

list_t *getBackground(processHandler_t const *processHandler) {
    return processHandler->background;
}