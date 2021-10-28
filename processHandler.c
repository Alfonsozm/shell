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
    //TODO
    removeForeground(processHandler);
    free(processHandler);
}

void addForeground(processHandler_t *processHandler, process_t *process) {
    processHandler->foreground = process;
}

process_t *getForeground(processHandler_t *processHandler) {
    return processHandler->foreground;
}

void removeForeground(processHandler_t *processHandler) {
    cleanProcess(processHandler->foreground);
    processHandler->foreground = NULL;
}

int addBackground(processHandler_t *processHandler, process_t *process){
    return addInfo(processHandler->background, process);
}