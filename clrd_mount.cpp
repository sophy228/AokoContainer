/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <stddef.h>
#include <string.h>

//#include <android-base/logging.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "clrd_selinux.h"

struct mount_info{
const char * device;
const char * target;
const char * type;
unsigned long flag;
const void * data; 
};


#define MOUNT_CGROUP

static struct mount_info mountlist[] = {
   {
        "none",
        "/proc",
        "proc",
        0,
        NULL,
    },
    {
        "none",
        "/sys",
        "sysfs",
        0,
        NULL,
    },
#ifdef MOUNT_CGROUP
    {
        "cgroup_root",
        "/sys/fs/cgroup",
        "tmpfs",
        0,
        NULL,
    },
    {
        "cgroup",
        "/sys/fs/cgroup/cpuacct",
        "cgroup",
        MS_NODEV | MS_NOEXEC | MS_NOSUID,
        "cpuacct"
    },
    {
        "cgroup",
        "/sys/fs/cgroup/cpu",
        "cgroup",
        MS_NODEV | MS_NOEXEC | MS_NOSUID,
        "cpu"
    },
#endif
    {
        "/dev",
        "/dev",
        "",
        MS_BIND,
        NULL,
    },
#ifdef ANDROID
    {
        "/vendor/lib/modules",
        "/lib/modules",
        "",
        MS_BIND,
        NULL,
    },
    {
        "/system/",
        "/system/",
        "",
        MS_BIND | MS_REC,
        NULL,
    },
    {
        "/vendor",
        "/vendor",
        "",
        MS_BIND | MS_REC,
        NULL,
    },
    {
        "/apex",
        "/apex",
        "",
        MS_BIND | MS_REC,
        NULL,
    },
#else
    {
        "/etc/resolv.conf",
        "/etc/resolv.conf",
        "",
        MS_BIND | MS_RDONLY,
        NULL,
    },
#endif
};

static void mkdir_if_no_exist(char * dir) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}
int mount_fs(const char * parent) {

    int listlen = sizeof(mountlist) / sizeof(mountlist[0]);
    for(int i = 0; i < listlen; i++) {
        struct mount_info mi = mountlist[i];
        char * target;
        if(parent)
            asprintf(&target, "%s%s", parent, mi.target);
        else
            asprintf(&target, "%s", mi.target);

        mkdir_if_no_exist(target);
#ifdef ANDROID
        change_file_context(target, "u:object_r:clrd_mnt_dir:s0");
#endif
        int ret = mount(mi.device, target, mi.type, mi.flag, mi.data);
        if(ret) {
           // PLOG(ERROR) << "mount device error at " << target;
        }
        if(target)
            free(target);
    }
    return 0;
}

static int umount_all_fs(const char * parent) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    char *targetlist[512];
    int listleng= 0;

    fp = fopen("/proc/self/mountinfo", "r");
    if (fp == NULL) {
        printf("can't open  mountinfo\n");
        return -1;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        int num1, num2;
        char  dev[len];
        char  root[len];
        char  target[len];
        //printf("line:%s", line);
        sscanf(line, "%d %d %s %s %s", &num1, &num2, (char *)&dev, (char *)&root, (char *)&target);
        if(strncmp(target, parent, strlen(parent)) == 0) {
          //  printf("Retrieved target:%s \n", target);
            asprintf(&targetlist[listleng], "%s", target);
            listleng ++ ;
        }
        free(line);
        line = NULL;
    }
    for(int i = listleng -1 ; i >= 0; i --) {
        int ret = umount2(targetlist[i], MNT_FORCE);
        printf("umount2 %s\n", targetlist[i]);
        if(ret) {
           // PLOG(ERROR) << "umount device error at " << targetlist[i];
            printf("umount2 failed %s\n", targetlist[i]);
        }
        free(targetlist[i]);
    }
    return 0;
}

int unmount_fs(const char * parent){
    umount_all_fs(parent);
    printf("umount_all_fs done \n");
   /*  int listlen = sizeof(mountlist) / sizeof(mountlist[0]);
    for(int i = listlen - 1; i >= 0; i--) {
        struct mount_info mi = mountlist[i];
        char * target;
        if(parent)
            asprintf(&target, "%s%s", parent, mi.target);
        else
            asprintf(&target, "%s", mi.target); 
        int ret = umount2(target, MNT_FORCE);
        printf("umount2 %s\n", target);
        if(ret) {
            PLOG(ERROR) << "umount device error at " << target;
            printf("umount2 failed %s\n", target);
        }
        if(target)
            free(target);
    }*/
   return 0;
}




