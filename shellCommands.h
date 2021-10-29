#ifndef shell_commands_h
#define shell_commands_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int cd(char* dir);

int jobs();

int foreground(int i);

#endif
