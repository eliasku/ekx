#include <ek/local_res.h>

bool ek_local_res_success(const ek_local_res* lr) {
    return lr && lr->status == 0;
}

void ek_local_res_close(ek_local_res* lr) {
    if (lr && lr->closeFunc) {
        lr->closeFunc(lr);
    }
}

#if defined(__ANDROID__)

#include "local_res_android.c.h"

#elif defined(__EMSCRIPTEN__)

#include "local_res_web.c.h"

#else

#include "local_res_sys.c.h"

#endif

#ifndef __EMSCRIPTEN__

void ek_local_res_load(const char* path, ek_local_res_callback callback, void* userdata) {
    ek_local_res lr = {0};
    lr.userdata = userdata;
    ek_local_res_get_file_platform(path, &lr);
    callback(&lr);
}

#endif
