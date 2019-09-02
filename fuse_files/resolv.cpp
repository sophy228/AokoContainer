
#include "clrd_fuse.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include <unistd.h>
#include <cutils/properties.h>
//#include <android-base/logging.h>

#define FILE_NAME "resolv.conf"
#define DEST_FILE_PATH "/etc/resolv.conf"

static struct fuse_file_hanlder * my_handler;
static int read_resolve(char * buffer, size_t size, size_t offset) {

    int i, len;
    char content[4096];
    int index = 0;
   // printf("read conf:%d %d", size, offset);
   // PLOG(INFO)<< "read_resolve size: " << size << "offset " << offset ;
    for(i = 1 ; i <= 3; i++) {
        char * dns_prop;
        char prop[PROP_VALUE_MAX] = "";
        asprintf(&dns_prop,"net.dns%d", i);
        property_get(dns_prop, prop,  "");
        if(strlen(prop)) {
            int ret = sprintf(content+index, "nameserver %s\n", prop);
            index+= ret;
        }
        free(dns_prop);
    }
    len = strlen(content);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buffer, content + offset, size);
	} else
		size = 0;
    return size;
}

static int getsize_resolve() {
    int i, len;
    char content[4096];
    int index = 0;
    for(i = 1 ; i <= 3; i++) {
        char * dns_prop;
        char prop[PROP_VALUE_MAX] = "";
        asprintf(&dns_prop,"net.dns%d", i);
        property_get(dns_prop, prop,  "");
        if(strlen(prop)) {
            int ret = sprintf(content+index, "nameserver %s\n", prop);
            index+= ret;
        }
        free(dns_prop);
    }
    len = strlen(content);
    return len;
}

static void init() {
    if(my_handler != NULL) {
        return;
    }
    my_handler = new struct fuse_file_hanlder;
    strcpy(my_handler->filename, FILE_NAME);
    my_handler->read = read_resolve;
    my_handler->getsize = getsize_resolve;

    unlink(DEST_FILE_PATH);
    char srcfilename[128];
    sprintf(srcfilename, "/%s/%s", FUSE_MOUNT_POINT, FILE_NAME);
	symlink(srcfilename, DEST_FILE_PATH);
}

struct fuse_file_hanlder *  get_resolve_handle() {
    init();
    return my_handler;
}
