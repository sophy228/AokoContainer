
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
static char *nsnames = "user\0uts\0pid\0net\0mnt\0ipc";

static int ns_enter(int pid) {
    char filename[FILENAME_MAX];
    int fd;
    int i = 0;
    int listlen = sizeof(flags) / sizeof(flags[0]);
    while(i < listlen) {
         
         sprintf(filename, "/proc/%ld/ns/%s", pid, nsnames);
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


