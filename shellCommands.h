#ifndef shell_commands_h
#define shell_commands_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "processHandler.h"

/**
 * Decide which internal command to call (if any).
 *
 * @param processHandler the specific processHandler_t.
 * @param command the specific tcommand.
 * @param cwd the current working directory. If a change to the working directory is made this parameter is updated.
 */
void shellCommand(processHandler_t *processHandler, tcommand *command, char* cwd);

/**
 * Changes the current working directory
 * @param dir the new directory the user wants to change to (if its value is NULL then the new directory will be HOME).
 * @return an integer. 0 if the directory has change correctly or -1 if the new directory was incorrect.
 */
int cd(char const *dir);

/**
 * Prints the job id and status of all the processes at background.
 * @param processHandler the specific processHandler_t.
 */
void jobs(processHandler_t const *processHandler);

/**
 * Brings processes at background to foreground and resumes its execution if stopped.
 * @param processHandler the specific processHandler_t.
 * @param jobId the specific job id. If the JobId is zero the function will bring the last process.
 */
void foreground(processHandler_t *processHandler, int jobId);

/**
 * Resumes the execution of the process at the background.
 * @param processHandler the specific processHandler_t.
 * @param jobId the specific job id. If the JobId is zero the function will send the last process.
 */
void background(processHandler_t *processHandler, int jobId);

#endif
