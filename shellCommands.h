#ifndef shell_commands_h
#define shell_commands_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "processHandler.h"

int cd(char const *dir);

void jobs(processHandler_t const *processHandler);

void foreground(processHandler_t *processHandler, int i);

void background(processHandler_t *processHandler, int i);

#endif
