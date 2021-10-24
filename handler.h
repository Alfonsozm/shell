#ifndef handler_h
#define handler_h

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "processHandler.h"

void handleSIGINT(int sig);

void handleSIGQUIT(int sig);

void handleSIGUSR1(int sig);

#endif