#include "processHandler.h"

static process_t background[256];
static int count;
static process_t foreground;

void addForeground(process_t process) {
    foreground = process;
}

char *getForegroundLine() {
    return foreground.line;
}

int getForegroundCount() {
    return foreground.count;
}

pid_t *getForegroundPid() {
    return foreground.pid;
}

int foregroundContainsPid(pid_t pid) {
    int bool = 0;
    for (int i = 0; i < foreground.count; ++i) {
        bool = bool || (foreground.pid[i] == pid);
    }
    return bool;
}

void removeForeground() {
    free(foreground.pid);
    free(foreground.line);

    foreground.line = NULL;
    foreground.pid = NULL;
}