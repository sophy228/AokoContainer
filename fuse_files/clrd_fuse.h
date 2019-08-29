#ifndef CLRD_FUSE
#define CLRD_FUSE 1
#include <stddef.h>
#define FUSE_MOUNT_POINT "clrdfs"
int create_clrd_fs(char * target);

struct fuse_file_hanlder {
char filename[4096];
int (* read)(char *buf, size_t size, size_t offset);
int (* getsize)(void);
struct fuse_file_hanlder * next;
};
#endif