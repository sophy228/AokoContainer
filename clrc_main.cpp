//#include <android-base/logging.h>
//#include <cutils/properties.h>

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>


#include "lib/ns_lib.h"

static void setup_env() {
    //clearenv();
    putenv((char *)"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:");
    char * home = NULL;
    asprintf(&home, (char*)"HOME=/%s", getlogin());
    if(home) {
        putenv(home);
        free(home);
    }
}

int main(int argc, char * argv[]) {
    (void)argc;
    enter_clrd_space();
    setup_env();
    char * home = getenv((char*)"HOME");
    if(home)
        chdir(home);
    else
    {
         chdir("/");
    }
    
    if(fork() == 0) {
        printf("exec %s\n", argv[1]);
        execvp(argv[1], &argv[1]);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    if(!sigemptyset(&sa.sa_mask)) {
        sa.sa_handler = SIG_IGN;
        sigaction(SIGINT, &sa, NULL);
    }
    waitpid(-1, NULL, 0);
    return 0;
}