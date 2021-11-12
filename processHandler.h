#ifndef processHandler_h
#define processHandler_h

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "list.h"

typedef enum {
    RUNNING,
    STOPPED,
    ENDED
} status_t;

typedef struct process_t {
    char *line;
    int jobId;
    int count;
    pid_t *pid;
    status_t *pidStatus;
    pid_t groupPid;
    status_t groupStatus;
    pid_t *ioHandlers;
    int *hasRedirection;
} process_t;

typedef struct processHandler_t {
    struct list_t *background;
    int totalBackgroundProcessesAdded;
    struct process_t *foreground;
} processHandler_t;

processHandler_t *createEmptyProcessHandler();

process_t *createNewProcess(char *line, int id, int count, pid_t *pid, pid_t *ioHandler, int *hasRedirection);

void checkProcessStatus(process_t *p);

//frees all the memory associated with the specified process_t
void cleanProcess(process_t *process);

//frees all the memory associated with the specified processHandler_t
void cleanProcessHandler(processHandler_t *processHandler);

void addForeground(processHandler_t *processHandler, process_t *process);

process_t *getForeground(processHandler_t const *processHandler);

//frees all the memory of the process_t in processHandler.foreground and sets the value to NULL
void removeForeground(processHandler_t *processHandler);

int addBackground(processHandler_t *processHandler, process_t *process);

process_t *removeBackground(processHandler_t *processHandler, int jobId);

list_t *getBackground(processHandler_t const *processHandler);

process_t *getBackgroundByJobId(processHandler_t const *processHandler, int jobId);

#endif