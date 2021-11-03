#include "processHandler.h"

processHandler_t *createEmptyProcessHandler() {
    processHandler_t *processHandler = (processHandler_t *) malloc(sizeof(processHandler_t));
    processHandler->foreground = NULL;
    processHandler->background = createEmptyList();
    return processHandler;
}

void cleanProcess(process_t *process) {
    free(process->line);
    free(process->pid);
    free(process);
}

void cleanProcessHandler(processHandler_t *processHandler) {
    node_t *n = processHandler->background->first;
    while (n != NULL) {
        cleanProcess((process_t *) n->info);
        n = n->next;
        free(n->previous);
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
    int i = addInfo(processHandler->background, process);
    if (process->id == -1) {
        process->id = i;
    }
    return i;
}