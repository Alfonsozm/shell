#include "shellCommands.h"

int cd(char *dir){
    int check;
    if(*dir == NULL) {
        check = chdir(getenv("HOME"));
    } else {
        check = chdir(*dir);
    }

    if(check==0){
        char s[100];
        fprintf(stdout,"Actual directory: %s %s", getenv("PWD"), getcwd(s,100));
        return 0;
    }else{
        fprintf(stderr, "Incorrect directory name\n");
        return -1;
    }
}