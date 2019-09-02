#ifndef LOOP_DEV
#define LOOP_DEV 1
#include <stdint.h>
int create_loop_dev(const char * image_file, uint64_t offset, char ** loop_device = NULL);
int destroy_loop_dev(char * loop_device);
#endif