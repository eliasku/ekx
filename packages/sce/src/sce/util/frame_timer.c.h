#include <sce/util/frame_timer.h>
#include <ek/app.h>
#include <ek/time.h>
#include <stdbool.h>

frame_timer_t frame_timer;

static bool frame_timer_update_from_display_ts(double* delta) {
    if (ek_app.frame_callback_timestamp > 0) {
        const double ts = ek_app.frame_callback_timestamp;
        const double prev = frame_timer.app_fts_prev;
        frame_timer.app_fts_prev = ts;
        if (prev > 0.0) {
            *delta = ts - prev;
            return true;
        }
    }
    return false;
}

void update_frame_timer(void) {
    // anyway we need update stopwatch state, it could be useful for another functions
    double dt = ek_ticks_to_sec(ek_ticks(&frame_timer.stopwatch));
    // if available, upgrade delta with timestamp from app's display-link
    frame_timer_update_from_display_ts(&dt);
    frame_timer.dt = dt;
    ++frame_timer.idx;
}

