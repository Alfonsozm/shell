#ifndef processHandler_h
#define processHandler_h

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "list.h"
#include "parser.h"

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

/**
 * Creates and reserves memory for an empty processHandler_t.
 *
 * @return a pointer to the newly created processHandler_t.
 */
processHandler_t *createNewProcessHandler_T();

/**
 * Creates and reserves memory for a process_t with the specified parameters.
 *
 * @param line the commands and argument that represent this process_t.
 * @param id the job id of this process_t.
 * @param count the amount of processes inside this process_t.
 * @param pid the array of pid_t of the processes inside this process_t.
 * @param ioHandler the pid_t of the ioHandler processes for this process_t.
 * @param hasRedirection an array representing if this process_t has any redirections.
 * @return the pointer to the newly created process_t.
 */
process_t *createNewProcess_T(char *line, int id, int count, pid_t *pid, pid_t *ioHandler, int *hasRedirection);

/**
 * Updates the status of a specific process_t
 *
 * @param p the specific process_t
 */
void checkProcessStatus(process_t *p);

/**
 * Frees all the memory associated with the specific process_t.
 *
 * @param process the specific process_t.
 */
void cleanProcess_T(process_t *process);

/**
 * Frees all the memory associated with the specific processHandler_t.
 *
 * @param processHandler the specific processHandler_t.
 */
void cleanProcessHandler_T(processHandler_t *processHandler);

/**
 * Sets processHandler.foreground to a specific process_t.
 *
 * @param processHandler the specific processHandler_t.
 * @param process the specific process_t.
 */
void setForeground(processHandler_t *processHandler, process_t *process);

/**
 * Returns the process_t in processHandler.foreground.
 *
 * @param processHandler the specific processHandler_t.
 * @return the specific process_t.
 */
process_t *getForeground(processHandler_t const *processHandler);

/**
 * Cleans the process_t in processHandler.foreground and sets its value value to NULL.
 *
 * @param processHandler the specific processHandler_t.
 */
void removeForeground(processHandler_t *processHandler);

/**
 * Adds a process_t to processHandler.background.
 *
 * @param processHandler the specific processHandler_t.
 * @param process the specific process_t.
 * @return the new job id of the process_t.
 */
int addBackground(processHandler_t *processHandler, process_t *process);

/**
 * Removes a process_t from processHandler.background given its job id.
 *
 * @param processHandler the specific processHandler_t.
 * @param jobId the specific job id.
 * @return the removed process_t.
 */
process_t *removeBackground(processHandler_t *processHandler, int jobId);

/**
 * Returns the list_t in processHandler.background.
 *
 * @param processHandler the specific processHandler_t.
 * @return the specific list_t.
 */
list_t *getBackground(processHandler_t const *processHandler);

#endif