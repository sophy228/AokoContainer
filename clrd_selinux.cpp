/*Copyright (C) 2019 Zhongmin Wu

License - MIT
=============

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <selinux/android.h>
#include <selinux/selinux.h>
#include <android-base/logging.h>
#include <sys/stat.h>


static struct selabel_handle* sehandle = NULL;
void change_file_context(const char * file, const char *context) {
    setfilecon(file, context);
}
void setup_selinux_handle() {
    sehandle = selinux_android_file_context_handle();
    printf("get sehandle:%p\n",sehandle);
    if(sehandle)
        selinux_android_set_sehandle(sehandle);
}

int setup_selinux_context(char * path) {
    char * secontext = nullptr;
    char * actcontext = nullptr;
    int ret = setexeccon("u:r:init:s0");
    if(ret < 0) {
        PLOG(ERROR) << "setexeccon to init error"  ;
    }
    if (sehandle) {
        if (!selabel_lookup(sehandle, &secontext, path, S_IFDIR)) {
            PLOG(INFO) << "selabel_lookup  " << path << " " <<  secontext;
            getfilecon(path, &actcontext);
            PLOG(INFO) << "getfilecon  " << path << " " <<  actcontext;
           
        }

        //selabel_lookup(sehandle, &secontext, "/lib/systemd/systemd", S_IFREG);
       // PLOG(INFO) << "selabel_lookup  " << "/lib/systemd/systemd" << " " <<  secontext;

    }
    if (strcmp(actcontext, secontext) && !selinux_android_restorecon(path, SELINUX_ANDROID_RESTORECON_RECURSE|SELINUX_ANDROID_RESTORECON_FORCE)) {
            PLOG(INFO) << "restorecon at " << path ;
    }
    freecon(actcontext);
    freecon(secontext);
    return 0;
}

int selinux_check_pass() {
    if(!is_selinux_enabled())
        return 1;
    return !security_getenforce();
}