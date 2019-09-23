#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>   // new code: include mount.h
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <linux/sched.h>

#include "clrd_child_main.h"

#define STACK_SIZE (5*1024 * 1024)
static char child_stack[STACK_SIZE];

static int flags = CLONE_NEWNS
//		| CLONE_NEWCGROUP
//		| CLONE_NEWPID
		| CLONE_NEWIPC
//		| CLONE_NEWNET
		| CLONE_NEWUTS;

int create_new_namespace(struct child_config* configs) {
  if(configs->new_pid_ns)
    flags |= CLONE_NEWPID;
  int child_pid = clone(child_main, child_stack+STACK_SIZE,
    flags | SIGCHLD, configs);
  return child_pid;
}

int clear_namespace(struct child_config* configs) {
    destroy_child(configs);
    return 0;
}
