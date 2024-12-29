#include <string.h>
#include <sys/utsname.h>
#include "utils.h"

int is_linux() {
    struct utsname uname_data;
    int ret_uname;

    ret_uname = uname(&uname_data);
    if (ret_uname < 0) {
        return 0;
    }

    if (strcmp(uname_data.sysname, "Linux") == 0) {
        return 1;
    } else {
        return 0;
    }
}
