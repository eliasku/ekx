#ifndef SCENEX_BASIC_APPLICATION_H
#define SCENEX_BASIC_APPLICATION_H

#include <ecx/ecx.h>
#include <ekx/app/frame_timer.h>
#include <ekx/app/game_display.h>
#include <ek/app.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (* pre_load)(void);

    // after root pack resources are loaded
    void (* start)(void);

    void (* update)(void);

    void (* event)(ek_app_event event);

    void (* pre_render)(void);

    void (* pre_render_scene)(void);

    void (* post_render_scene)(void);

    void (* terminate)(void);
} game_delegate_t;

typedef struct {
    game_display display;
    frame_timer_t frame_timer;
    float scale_factor;
    entity_t root;
    game_delegate_t delegate;
    bool started;
} game_app_state_t;

extern game_app_state_t game_app_state;

void run_app(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_BASIC_APPLICATION_H
