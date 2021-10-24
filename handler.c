#include "handler.h"

void handleSIGINT(int sig) {
    if (foregroundContainsPid(getpid())) {
        exit(0);
    }
}

void handleSIGQUIT(int sig) {
    if (foregroundContainsPid(getpid())) { //core
        exit(0);
    }
}

void handleSIGUSR1(int sig) {

}