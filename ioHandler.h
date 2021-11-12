#ifndef ioHandler_h
#define ioHandler_h

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

typedef enum {
    IN,
    OUT,
    ERR
} io_t;

typedef enum {
    ON,
    OFF
} on_off_t;

int stdinBACKUP;
int stdoutBACKUP;
int stderrBACKUP;
int devNULL;

static io_t ioType;
static on_off_t status;

void init();

pid_t createNewIOHandler(int pipeIn, int pipeOut, io_t io);

pid_t createNewIOHandlerOFF(int pipeIn, int pipeOut, io_t io);

void sigusr1Handler(int sig);

void sigusr2Handler(int sig);

#endif
