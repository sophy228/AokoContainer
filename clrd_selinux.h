#ifndef SELINUX_H
#define SELINUX_H 1
void change_file_context(const char * file, const char *context);
int setup_selinux_context(char * path) ;
void setup_selinux_handle();
int selinux_check_pass();
#endif