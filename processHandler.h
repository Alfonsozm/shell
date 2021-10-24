#ifndef processHandler_h
#define processHandler_h

#include <unistd.h>
#include <stdlib.h>

typedef struct {
    char *line;
    int count;
    pid_t *pid;
} process_t;

void addForeground(process_t process);

process_t getForeground();

char *getForegroundLine();

int getForegroundCount();

pid_t *getForegroundPid();

int foregroundContainsPid(pid_t pid);

void removeForeground();

#endif