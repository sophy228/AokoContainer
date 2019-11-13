/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
//#include <android-base/logging.h>

#include "lib/loopdev.h"

int create_loop_dev(const char * image_file, uint64_t offset, char ** loop_device) {    
    char * free_loop = loopdev_find_unused();
    if(loopdev_setup_device(image_file, offset, free_loop)) {
       // PLOG(ERROR) << "Failed to associate loop device " << free_loop << " to file " << image_file;
        printf("Failed to associate loop device %s to file %s\n", free_loop, image_file);
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