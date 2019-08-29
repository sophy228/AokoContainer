/*Copyright (C) 2012 Alex Chamberlain

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/loop.h>

#include <android-base/logging.h>

#include "loopdev.h"
#include "privileges.h"

static const char LOOPDEV_PREFIX[]   = "/dev/block/loop";
static int        LOOPDEV_PREFIX_LEN = sizeof(LOOPDEV_PREFIX)/sizeof(LOOPDEV_PREFIX[0])-1;

char * loopdev_find_unused() {
  int control_fd = -1;
  int n = -1;

  if(escalate()) return NULL;

  if((control_fd = open("/dev/loop-control", O_RDWR)) < 0) {
    PLOG(ERROR) << "Failed to open /dev/loop-control";
    return NULL;
  }

  if(drop()) return NULL;

  n = ioctl(control_fd, LOOP_CTL_GET_FREE);

  if(n < 0) {
    PLOG(ERROR) << "Failed to find a free loop device.";
    return NULL;
  }
  
  int l = strlen(LOOPDEV_PREFIX) + 1 + 1; /* 1 for first character, 1 for NULL */
  {
    int m = n;
    while(m /= 10) {
      ++l;
    }
  }

  char * loopdev = (char*) malloc(l * sizeof(char));
  sprintf(loopdev, "%s%d", LOOPDEV_PREFIX, n);

  return loopdev;
}

int loopdev_setup_device(const char * file, uint64_t offset, const char * device) {
  int file_fd = open(file, O_RDWR);
  int device_fd = -1;

  struct loop_info64 info;

  if(file_fd < 0) {
    PLOG(ERROR) << "Failed to open backing file " << file;
    goto error;
  }

  if(escalate()) goto error;

  if((device_fd = open(device, O_RDWR)) < 0) {
    PLOG(ERROR) << "Failed to open device " << device;
    goto error;
  }

  if(drop()) goto error;

  if(ioctl(device_fd, LOOP_SET_FD, file_fd) < 0) {
    PLOG(ERROR) <<  "Failed to set fd.";
    goto error;
  }

  close(file_fd);
  file_fd = -1;

  memset(&info, 0, sizeof(struct loop_info64)); /* Is this necessary? */
  info.lo_offset = offset;
  /* info.lo_sizelimit = 0 => max available */
  /* info.lo_encrypt_type = 0 => none */

  if(ioctl(device_fd, LOOP_SET_STATUS64, &info)) {
    PLOG(ERROR) << "Failed to set info.";
    goto error;
  }

  close(device_fd);
  device_fd = -1;

  return 0;

  error:
    if(file_fd >= 0) {
      close(file_fd);
    }
    if(device_fd >= 0) {
      ioctl(device_fd, LOOP_CLR_FD, 0);
      close(device_fd);
    }
    return 1;
}

int loopdev_unset_device(const char * device) {
  int device_fd = -1;
  struct loop_info64 info;

  if(escalate()) goto error;
  if((device_fd = open(device, O_RDWR)) < 0) {
    PLOG(ERROR) << "Failed to open device " << device;
    goto error;
  }

  if(drop()) goto error;

  memset(&info, 0, sizeof(struct loop_info64)); /* Is this necessary? */
  /* info.lo_sizelimit = 0 => max available */
  /* info.lo_encrypt_type = 0 => none */

  if(ioctl(device_fd, LOOP_SET_STATUS64, &info)) {
    PLOG(ERROR) << "Failed to set info.";
    goto error;
  }

error:
  if(device_fd >= 0) {
      ioctl(device_fd, LOOP_CLR_FD, 0);
      close(device_fd);
      return 0;
  }
  return 1;
}


