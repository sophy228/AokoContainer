/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <linux/capability.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#include "clrd_linuxinit.h"
#include "clrd_selinux.h"
#include "fuse_files/clrd_fuse.h"

//#include <android-base/logging.h>
#include "clrd_child_main.h"
char* const exec_args[] = {
(char *) "/bin/systemd",
(char *) "--log-level=debug",
(char *)"--log-target=kmsg",
(char *) "--log-location"
};

char* const exec_bashargs[] = {
  (char *)"/bin/bash",
  NULL
};

static int pivot_root(const char *new_root, const char *put_old)
{
       return syscall(SYS_pivot_root, new_root, put_old);
}

static void mkdir_if_no_exist(char * dir) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}

static void setup_env() {
    //clearenv();
    putenv((char *)"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:");
    char * home = NULL;
    asprintf(&home, "HOME=/%s", getlogin());
    if(home) {
        putenv(home);
        free(home);
    }
}

static void capbilities() {
   int ret = prctl(PR_CAPBSET_READ, CAP_SYS_ADMIN, 0, 0, 0);
   printf("if current ns has CAP_SYS_ADMIN:%d\n", ret);
}

int child_main(void * configs) {

    struct child_config * config = (struct child_config *)configs;
    char * loop_dev = config->loop_dev;
    char * target_dir = config->target_dir;
    char * old_rootdir;
    //int pid;
    //int status;
    capbilities();
    asprintf(&old_rootdir, "%s/%s", target_dir, "old_root");
    printf("mount loop device to %s\n",target_dir);

    if(mount(loop_dev, target_dir, "ext4", 0, NULL) != 0) {
        printf("Failed to mount loop device %s to %s\n", loop_dev, target_dir);
        goto error;
    }

    //PLOG(INFO) << "mount rootfs device:" << loop_dev << " on " << target_dir;

#if defined(__ANDROID__) && defined(__SELINUX__)
    setup_selinux_handle();
#endif
    /*pid = fork();

    if(pid == 0) {
       // PLOG(INFO) << "chroot to:" << target_dir;
        chroot(target_dir);
       // PLOG(INFO) << "resetlinux context:";
        chdir("/");
        setup_selinux_context("/");
        exit(0);
    } else 
    {
        
        waitpid(pid, &status,0);
    }*/

    init_rootfs(target_dir);

    mkdir_if_no_exist(old_rootdir);
    if(pivot_root(target_dir, old_rootdir)) {
      //  PLOG(ERROR)<< "Failed to pivot_root " << target_dir << " from " << old_rootdir;
       // PLOG(INFO) << "chroot to:" << target_dir;
        chroot(target_dir);
    }
    
    chdir("/");

#if defined(__ANDROID__) && defined(__SELINUX__)
    setup_selinux_context((char *)"/");
    create_clrd_fs((char *)"/");
#endif
    if(config->hostname)
        sethostname(config->hostname, strlen(config->hostname));
    
    setup_env();

    if(config->no_systemd || !config->new_pid_ns) {
        printf("exec %s \n", exec_bashargs[0]);
        execv(exec_bashargs[0], exec_bashargs);
    } else { 
        printf("exec %s \n", exec_args[0]);
        execv(exec_args[0], exec_args);
    }
   
 error:   
    //destroy_loop_dev(loop_dev);
    
    return 0;
}

int destroy_child(void * configs) {
    struct child_config * config = (struct child_config *)configs;
    deinit_rootfs(config->target_dir);
    printf("umount rootfs %s\n", config->target_dir);
    if(umount(config->target_dir) != 0) {
     //   PLOG(ERROR)<< "Failed to umount loop device " << config->loop_dev << " to mount point " << config->target_dir;
    }
    return 0;
}