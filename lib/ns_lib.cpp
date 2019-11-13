/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <android-base/logging.h>
#include <cutils/properties.h>

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <signal.h>


#define LINUX_PID_PROP  "sys.linux.pid"
#define LINUX_ROOTDIR_PROP  "sys.linux.rootdir"

static unsigned flags[]={CLONE_NEWUSER, CLONE_NEWUTS, CLONE_NEWPID, CLONE_NEWNET,
                   CLONE_NEWNS, CLONE_NEWIPC};
static char *nsnames = (char *)"user\0uts\0pid\0net\0mnt\0ipc";

static int ns_enter(int pid) {
    char filename[FILENAME_MAX];
    int fd;
    int i = 0;
    int listlen = sizeof(flags) / sizeof(flags[0]);
    while(i < listlen) {
         
         sprintf(filename, "/proc/%d/ns/%s", pid, nsnames);
         fd = open(filename, O_RDONLY);
         if (fd == -1) {
             printf("can not open %s\n", filename);
         }
         if(setns(fd, flags[i])) {
             printf("can not setns %s\n", filename);
         }

        printf("enter in to ns %s :%d\n", filename,  flags[i]);

         close(fd);
         nsnames += strlen(nsnames)+1;
         i++;
    }
    return 0;
}

int enter_clrd_space() {
    char prop[PROP_VALUE_MAX] = "";
    int pid = 0;
    char * rootdir;

    property_get(LINUX_PID_PROP, prop,  NULL);
    if(strlen(prop) <= 0 ) {
        printf("Linux runtime is not launched \n");
        goto error1;
    }

    pid = atoi(prop);
    printf("pid:%s %d\n", prop, pid);
    

    property_get(LINUX_ROOTDIR_PROP, prop,  NULL);
    if(strlen(prop) <= 0) {
        printf("Linux root dir is not found \n");
        goto error1;
    }
    rootdir =  prop;
    ns_enter(pid);
    chroot(rootdir);
   // setup_env();
    return 0;
error1 :
    return 1;
}


