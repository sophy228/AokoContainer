#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "clrd_namespace.h"
#include "clrd_loop.h"
#include "clrd_selinux.h"


//#include <android-base/logging.h>
#ifdef ANDROID
#include <cutils/properties.h>
#define LINUX_PID_PROP  "sys.linux.pid"
#define LINUX_ROOTDIR_PROP  "sys.linux.rootdir"
#endif

#define HELP_STRING "clrd [-b][-p] -l <rootfs.image> -t <mount point>\n"

static void print_usage() {
    printf(HELP_STRING);
}
static int parse_args(int argc, char * argv[], struct child_config *config) {
    int option = 0;
    while ((option = getopt(argc, argv, "bhpl:t:")) != -1 ) {
		switch (option) {
        case 'l':
            asprintf(&config->image_path, "%s", optarg);
            break;
        case 'p': 
            config->new_pid_ns = true;
            break;
        case 'b': 
            config->no_systemd = true;
            break;
        case 't':
            asprintf(&config->target_dir, "%s", optarg);
            break;
        case 'h':
			goto usage;
            break;
		}
	}
    if(!config->image_path) goto usage;
    if(!config->target_dir) goto usage;
    return 0;
usage:
    print_usage();
    return 1;
    
}

int main(int argc, char * argv[]) {
    struct child_config config;

#ifdef ANDROID
    if(!selinux_check_pass()) {
        printf("disable selinux or setenforce 0 and retry\n");
        return -1;
    }
#endif

    memset(&config, 0, sizeof(config));
  
    char * loop_dev = NULL;
    int ret;

    if(parse_args(argc, argv, &config)) {
       // PLOG(ERROR) << HELP_STRING; 
        return -1;
    }

   
    ret = create_loop_dev(config.image_path, 0, &loop_dev);
    if(ret) {
      //  PLOG(ERROR) << "Create loop dev error";
        printf("Create loop dev error\n");
        return 1;
    }

    config.loop_dev = loop_dev;

    printf("create_new_namespace\n");
    int child_pid = create_new_namespace(&config);
    printf("wait for pid %d\n", child_pid);

#ifdef ANDROID
    char number[20];
    sprintf(number, "%d", child_pid);
    property_set(LINUX_PID_PROP, number);
    property_set(LINUX_ROOTDIR_PROP, config.target_dir);
#endif

    int status;
    waitpid(child_pid, &status, 0);
    printf("waited for pid %d:%d\n", child_pid,status);

#ifdef ANDROID
    property_set(LINUX_PID_PROP, "");
    property_set(LINUX_ROOTDIR_PROP, "");
#endif

    clear_namespace(&config);
    destroy_loop_dev(loop_dev);
    return 0;
}