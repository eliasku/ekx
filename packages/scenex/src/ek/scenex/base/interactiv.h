#ifndef SCENE_INTERACTIVE_H
#define SCENE_INTERACTIVE_H

#include <ek/app.h>
#include <ek/math.h>
#include <ek/hash.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POINTER_EVENT_DOWN H("pointer_down")
#define POINTER_EVENT_UP   H("pointer_up")
#define POINTER_EVENT_OVER H("pointer_over")
#define POINTER_EVENT_OUT  H("pointer_out")
#define POINTER_EVENT_TAP  H("pointer_tap")

typedef struct interactive {
    ek_mouse_cursor cursor;

    // TODO: Interactive::pointer is useless
    // global world space pointer, because of that maybe we don't dispatch this coordinate into components,
    // just use global pointer from Interaction System
    vec2_t pointer;
    entity_t camera;

    //bool enabled = true;
    // todo: check bubble was false by default
    bool bubble;
    bool back_button;
    // responsive state
    bool over;
    bool pushed;
    // events
    bool ev_over;
    bool ev_tap;
    bool ev_out;
    bool ev_down;
    bool ev_tap_back;
} interactive_t;

void interactive_init(void);
interactive_t* interactive_add(entity_t e);
interactive_t* interactive_get(entity_t e);
void interactive_handle(interactive_t* interactive, entity_t e, string_hash_t type);
void interactive_clear_all_events(void);

#ifdef __cplusplus
};
#endif

#endif // SCENE_INTERACTIVE_H