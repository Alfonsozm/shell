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

static int stdinBACKUP;
static int stdoutBACKUP;
static int stderrBACKUP;
static int devNULL;

static io_t ioType;
static on_off_t status;

/**
 * Initializes the values of the STDIN, STDOUT, and STDERR backups as well as creates the fd for /dev/null.
 */
void init();

/**
 * Creates a new process for the io handling of STDIN, STDOUT or STDERR of a child job.
 *
 * @param pipeIn the pipe from where the ioHandler process should read.
 * @param pipeOut the pipe to which the ioHandler should write.
 * @param io the type of process, either IN, OUT or ERR.
 * @return the pid of the ioHandler process.
 */
pid_t createNewIOHandler(int pipeIn, int pipeOut, io_t io);

/**
 * Creates a new process for the io handling of STDIN of a child job to be executed in background.
 *
 * @param pipeIn the pipe from where the ioHandler process should read.
 * @param pipeOut the pipe to which the ioHandler should write.
 * @return the pid of the ioHandler process.
 */
pid_t createNewIOHandlerOFF(int pipeIn, int pipeOut);

/**
 * Handles SIGUSR1
 * @param sig ignored
 */
static void sigusr1Handler(int sig);

/**
 * Handles SIGUSR2
 * @param sig ignored
 */
static void sigusr2Handler(int sig);

#endif
