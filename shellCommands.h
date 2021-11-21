#ifndef shell_commands_h
#define shell_commands_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "processHandler.h"

void shellCommand(processHandler_t *processHandler, tcommand *command, char* cwd);

int cd(char const *dir);

void jobs(processHandler_t const *processHandler);

void foreground(processHandler_t *processHandler, int jobId);

void background(processHandler_t *processHandler, int jobId);

#endif
