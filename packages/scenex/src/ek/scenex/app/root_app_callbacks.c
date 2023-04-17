#include "root_app_callbacks.h"

#include <ek/canvas.h>
#include <ek/log.h>
#include <ek/time.h>
#include <ek/audio.h>
#include <ecx/ecx.h>

void root_app_on_frame() {
    log_tick();
    ek_timers_update();
}

void root_app_on_event(const ek_app_event ev) {
    if (ev.type == EK_APP_EVENT_PAUSE) {
        auph_set_pause(AUPH_MIXER, true);
    } else if (ev.type == EK_APP_EVENT_RESUME) {
        auph_set_pause(AUPH_MIXER, false);
    } else if (ev.type == EK_APP_EVENT_CLOSE) {
        ek_app.on_frame = NULL;
        ek_app.on_event = NULL;

        terminate_game_app();

        ecx_shutdown();
        canvas_shutdown();
        auph_shutdown();
        ek_gfx_shutdown();
    }
}
