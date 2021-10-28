#ifndef processHandler_h
#define processHandler_h

#include <unistd.h>
#include <stdlib.h>

#include "list.h"

typedef struct process_t {
    char *line;
    int count;
    pid_t *pid;
} process_t;

typedef struct processHandler_t {
    struct list_t *background;
    struct process_t *foreground;
} processHandler_t;

processHandler_t *createEmptyProcessHandler();

//frees all the memory associated with the specified process_t
void cleanProcess(process_t *process);

//frees all the memory associated with the specified processHandler_t
void cleanProcessHandler(processHandler_t *processHandler);

void addForeground(processHandler_t *processHandler, process_t *process);

process_t *getForeground(processHandler_t *processHandler);

//frees all the memory of the process_t in processHandler.foreground and sets the value to NULL
void removeForeground(processHandler_t *processHandler);

int addBackground(processHandler_t *processHandler, process_t *process);

#endif