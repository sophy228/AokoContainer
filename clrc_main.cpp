/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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