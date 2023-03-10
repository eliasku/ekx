#ifndef AUPH_WEB_IMPL
#define AUPH_WEB_IMPL
#else
#error You should implement auph once
#endif

#include "auph.h"
#include <emscripten.h>

void auph_setup(void) {
    EM_ASM(auph.setup());
}

void auph_shutdown(void) {
    EM_ASM(auph.shutdown());
}

auph_buffer auph_load(const char* filepath, int flags) {
    int r = EM_ASM_INT(return auph.load(UTF8ToString($0), $1), filepath, flags);
    return (auph_buffer) {r};
}

auph_buffer auph_load_data(const void* data, int size, int flags) {
    int r = EM_ASM_INT(return auph.loadMemory(HEAPU8.subarray($0, $0 + $1), $2), data, size, flags);
    return (auph_buffer) {r};
}

#ifdef __cplusplus
extern "C"{
#endif

void* auph_read_to_buffer(void* userdata, auph_buffer_callback callback);

EMSCRIPTEN_KEEPALIVE void* auph_read_to_buffer(void* userdata, auph_buffer_callback callback) {
    static float s[8192 * 2 * sizeof(float)];
    static float lr[8192 * 2 * sizeof(float)];
    callback(userdata, s, 8192);
    for (uint32_t i = 0; i < 8192; ++i) {
        lr[i] = s[i * 2];
        lr[i + 8192] = s[i * 2 + 1];
    }
    return lr;
}

#ifdef __cplusplus
}
#endif

auph_buffer auph_load_callback(auph_buffer_callback callback, void* userdata) {
    int r = EM_ASM_INT(return auph.load_callback($0, $1), callback, userdata);
    return (auph_buffer) {r};
}

void auph_unload(auph_buffer buffer) {
    EM_ASM(auph.unload($0), buffer.id);
}

auph_voice auph_play(auph_buffer buffer, int gain, int pan, int pitch, int flags, auph_bus bus) {
    int r = EM_ASM_INT(return auph.voice($0, $1, $2, $3, $4, $5, $6), buffer.id, gain, pan, pitch, flags, bus.id);
    return (auph_voice) {r};
}

void auph_stop(int name) {
    EM_ASM(auph.stop($0), name);
}

void auph_set(int name, int param, int value) {
    EM_ASM(auph.set($0, $1, $2), name, param, value);
}

int auph_get(int name, int param) {
    int r = EM_ASM_INT(return auph.get($0, $1), name, param);
    return r;
}

int auph_vibrate(int duration_millis) {
    int r = EM_ASM_INT(return auph.vibrate($0), duration_millis);
    return r;
}
