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