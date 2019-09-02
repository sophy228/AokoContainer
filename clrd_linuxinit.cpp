
#include "clrd_mount.h"
//#include <android-base/logging.h>



int init_rootfs(const char * path ) {

    mount_fs(path);
    return 0;

}

int deinit_rootfs(const char * path ) {
    unmount_fs(path);
    return 0;
}

