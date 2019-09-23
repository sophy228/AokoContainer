#ifndef CHILD_MAIN
#define CHILD_MAIN 1
struct child_config
{
    char * target_dir;
    char * image_path;
    char * loop_dev;
    char * hostname;
    bool new_pid_ns;
    bool no_systemd;
};

int child_main(void * configs);
int destroy_child(void * configs);
#endif