#include <android-base/logging.h>
#include <cutils/properties.h>

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <signal.h>


#include "lib/ns_lib.h"

static void setup_env() {
    //clearenv();
    putenv("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:");
    char * home = NULL;
    asprintf(&home, "HOME=/%s", getlogin());
    if(home) {
        putenv(home);
        free(home);
    }
}

int main(int argc, char * argv[]) {

    enter_clrd_space();
    setup_env();
    char * home = getenv("HOME");
    if(home)
        chdir(home);
    else
    {
         chdir("/");
    }
    
    if(fork() == 0) {
        execvp(argv[1], &argv[1]);
    }
    sigignore(SIGINT);
    waitpid(-1, NULL, 0);
error1:
    return 0;
}