#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <sched.h>
#include <pthread.h>

#include "lib/ns_lib.h"
#include "clrd_fuse.h"
#include "resolv.h"
#include "usergroup.h"

#include <sys/prctl.h>


//#include <android-base/logging.h>


using namespace std;

static struct options {
	const char *filename;
	const char *contents;
} options;



//static map<char *, struct fuse_file_hanlder *> file_handles;
static  struct fuse_file_hanlder fuse_file_head;

static void add_file_handler(struct fuse_file_hanlder  * fh) {
	struct fuse_file_hanlder  * phead = &fuse_file_head;
	while(phead) {
		if(phead->next == NULL) {
			phead->next = fh;
			fh->next = NULL;
			break;
		}
		phead = phead->next;
	}
}

static struct fuse_file_hanlder * find_file_handler(const char * filename) {
	struct fuse_file_hanlder  * phead = fuse_file_head.next;
	while(phead) {
		if(strcmp(phead->filename, filename) == 0) {
			break;
		}
		phead = phead->next;
	}
	return phead;
}



static void *clrd_fuse_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	add_file_handler(get_resolve_handle());
	add_file_handler(get_usergroup_handle());
	return NULL;
}


static int clrd_fuse_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	//PLOG(INFO)<< "clrd_fuse_getattr " << path ;
	memset(stbuf, 0, sizeof(struct stat));

	if(struct fuse_file_hanlder * handler = find_file_handler(path+1)) {
		stbuf->st_mode = S_IFREG | 0744;
		stbuf->st_nlink = 1;
		stbuf->st_size = handler->getsize();
		return res;
	}
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;

	return res;
}

static int clrd_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

    enum fuse_fill_dir_flags flag = (enum fuse_fill_dir_flags)0;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, flag);
	filler(buf, "..", NULL, 0, flag);
	filler(buf, options.filename, NULL, 0, flag);
	struct fuse_file_hanlder  * phead = fuse_file_head.next;
	while(phead) {
		filler(buf, phead->filename, NULL, 0, flag);
		phead = phead->next;
	}
	return 0;
}


static int clrd_fuse_open(const char *path, struct fuse_file_info *fi)
{
	//printf("clrd_fuse_open %s\n", path);
//	PLOG(INFO)<< "clrd_fuse_open " << path ;
	if(struct fuse_file_hanlder * handler = find_file_handler(path+1)) {
		if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;
		return 0;
	}

	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int clrd_fuse_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	//printf("read %s\n", path);
	//PLOG(INFO)<< "clrd_fuse_read " << path ;
	if(struct fuse_file_hanlder * handler = find_file_handler(path+1)) {
		return handler->read(buf, size, offset);
	}
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations clrd_fuse_oper = {
	.init           = clrd_fuse_init,
	.getattr	= clrd_fuse_getattr,
	.readdir	= clrd_fuse_readdir,
	.open		= clrd_fuse_open,
	.read		= clrd_fuse_read,
};


static int create_fuse(int argc, char *argv[]) {
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");
    //fuse_opt_add_arg(&args, "-d");
	//fuse_opt_add_arg(&args, "-oallow_other");
	printf("fuse_main before\n");
    ret = fuse_main(args.argc, args.argv, &clrd_fuse_oper, NULL);
	printf("fuse_main after\n");
	fuse_opt_free_args(&args);	
	return ret;
}

static void mkdir_if_no_exist(char * dir) {
    struct stat st;
	memset(&st, 0, sizeof(st));
    printf("stat %s\n", dir);
    int ret = stat(dir, &st);
    if(ret == 0)
        printf("dir %s mode is %d\n", dir, st.st_mode);
    if (ret) {
       printf("mkdir %s\n", dir);
       int ret = mkdir(dir, 0700);
       if(ret){
         //  PLOG(ERROR) << "mkdir failed  " << dir;
           printf("mkdir failed\n");
       }
    }
}


#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];



//static char *argv[] = {"fuse_main", "-f", "clrdfs", "-oallow_other"};

static int fuse_child_main(void * data) {
	char * target = (char *)data;
    char * argv[4];

	asprintf(&argv[0], "%s", "fuse_clrd");
	asprintf(&argv[1], "%s/%s", target, FUSE_MOUNT_POINT);
	asprintf(&argv[2], "%s", "-f");
	asprintf(&argv[3], "%s",  "-oallow_other");


	pthread_setname_np(pthread_self(), argv[0]);

	mkdir_if_no_exist(argv[1]);

    int listlen = 4;
	create_fuse(listlen, argv);

   	return 0;
   //return 0;
}

int create_clrd_fs(char * target) {
    int child_pid = clone(fuse_child_main, child_stack+STACK_SIZE,
     SIGCHLD, target);
    return child_pid;
}






