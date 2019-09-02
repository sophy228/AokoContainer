
//#include <cutils/properties.h>
#include <android-base/logging.h>

#include "clrd_fuse.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

#define FILE_NAME "usergroup.sh"
#define DEST_FILE_PATH "/sbin/usergroup.sh"

static struct fuse_file_hanlder * my_handler;
static char *content = NULL;
static void build_content() {
    stringstream str;
    str << "##!/bin/sh \n\n\n";
   // str << "set -e\n\n";
    str << "usergroupadd() {\n grep -q ^$2: /etc/group || groupadd -r -g $1 $2 \n grep -q ^$2: /etc/passwd || useradd -r -g $1 -u $1 -d / $2 \n }\n";
    
    struct passwd *pwd;
    while ((pwd = getpwent()) != NULL) {
        if(pwd->pw_uid > 9999 || strncmp(pwd->pw_name, "oem", 3) == 0)
            continue;
        //PLOG(INFO)<< "usergroupadd " << pwd->pw_uid << " " << pwd->pw_name;
        str << "usergroupadd " << pwd->pw_uid << " " << pwd->pw_name << "\n";
    }
    asprintf(&content, "%s", str.str().c_str());
   // PLOG(INFO)<< content << "size = " << strlen(content);
}

static int read_handler(char * buffer, size_t size, size_t offset) {

    int len;
   // printf("read conf:%d %d", size, offset);
   // PLOG(INFO)<< "read_resolve size: " << size << "offset " << offset ;
    len = strlen(content);
    PLOG(INFO)<< "len size: " << len;
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buffer, content + offset, size);
	} else
		size = 0;
    return size;
}

static int getsize_handler() {
    int len;
    len = strlen(content);
 //   PLOG(INFO)<< "getsize_handler size: " << len;
    return len;
}

static void init() {
    if(my_handler != NULL) {
        return;
    }
    build_content();
    my_handler = new struct fuse_file_hanlder;
    strcpy(my_handler->filename, FILE_NAME);
    my_handler->read = read_handler;
    my_handler->getsize = getsize_handler;

    unlink(DEST_FILE_PATH);
    char srcfilename[128];
    sprintf(srcfilename, "/%s/%s", FUSE_MOUNT_POINT, FILE_NAME);
	symlink(srcfilename, DEST_FILE_PATH);
}

struct fuse_file_hanlder *  get_usergroup_handle() {
    init();
    return my_handler;
}
