#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <linux/capability.h>
#include <sys/wait.h>

#include "clrd_linuxinit.h"
#include "clrd_selinux.h"
#include "fuse_files/clrd_fuse.h"

#include <android-base/logging.h>
#include "clrd_child_main.h"
char* const exec_args[] = {
  "/bin/systemd",
  "--log-level=debug",
  "--log-target=kmsg",
  "--log-location"
};

char* const exec_bashargs[] = {
  "/bin/bash",
  NULL
};

static int pivot_root(const char *new_root, const char *put_old)
{
       return syscall(SYS_pivot_root, new_root, put_old);
}

static void mkdir_if_no_exist(char * dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}

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

static void capbilities() {
   int ret = prctl(PR_CAPBSET_READ, CAP_SYS_ADMIN, 0, 0, 0);
   printf("if current ns has CAP_SYS_ADMIN:%d\n", ret);
}

int child_main(void * configs) {

    struct child_config * config = (struct child_config *)configs;
    char * loop_dev = config->loop_dev;
    char * target_dir = config->target_dir;
    char * old_rootdir;
    int pid;
    int status;

    capbilities();
    
    asprintf(&old_rootdir, "%s/%s", target_dir, "old_root");
    printf("mount loop device to %s\n",target_dir);

    if(mount(loop_dev, target_dir, "ext4", 0, NULL) != 0) {
        PLOG(ERROR)<< "Failed to mount loop device " << loop_dev << " to mount point " << target_dir;
        goto error;
    }

    PLOG(INFO) << "mount rootfs device:" << loop_dev << " on " << target_dir;


    setup_selinux_handle();

    /*pid = fork();

    if(pid == 0) {
        PLOG(INFO) << "chroot to:" << target_dir;
        chroot(target_dir);
        PLOG(INFO) << "resetlinux context:";
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
        PLOG(ERROR)<< "Failed to pivot_root " << target_dir << " from " << old_rootdir;
        PLOG(INFO) << "chroot to:" << target_dir;
        chroot(target_dir);
    }
    
    chdir("/");
    setup_selinux_context("/");
    create_clrd_fs("/");
    
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
        PLOG(ERROR)<< "Failed to umount loop device " << config->loop_dev << " to mount point " << config->target_dir;
    }
    return 0;
}