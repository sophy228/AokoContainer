#include <stdio.h>
#include <stdlib.h>
#include <android-base/logging.h>

#include "lib/loopdev.h"

int create_loop_dev(const char * image_file, uint64_t offset, char ** loop_device) {    
    char * free_loop = loopdev_find_unused();
    if(loopdev_setup_device(image_file, offset, free_loop)) {
        PLOG(ERROR) << "Failed to associate loop device " << free_loop << " to file " << image_file;
        goto error;
    }
    if(loop_device != NULL)
        *loop_device = free_loop;
    return 0;
error:
    if(free_loop)
        free(free_loop);
    return 1;
}

int destroy_loop_dev(char * loop_device) {
    return loopdev_unset_device(loop_device);
}