/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
