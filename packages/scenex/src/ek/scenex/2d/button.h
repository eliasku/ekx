#ifndef SCENEX_BUTTON_H
#define SCENEX_BUTTON_H

#include <ecx/ecx.h>
#include <ek/hash.h>
#include <ek/math.h>
#include <ekx/app/time_layers.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON_EVENT_CLICK H("button_click")

typedef struct {
    string_hash_t sfx_over;
    string_hash_t sfx_down;
    string_hash_t sfx_click;
    string_hash_t sfx_out;
    string_hash_t sfx_cancel;

    float over_speed_forward;
    float over_speed_backward;
    float push_speed_forward;
    float push_speed_backward;
} button_skin_t;

typedef struct {
    button_skin_t* skin;

    float time_over;
    float time_push;
    float time_post;

    vec2_t base_skew;
    vec2_t base_scale;
    color2_t base_color;

    TimeLayer time;

    bool initialized;
    bool pushed;
} button_t;

extern ecx_component_type Button;
button_t button(void);
void Button_setup(void);
void Button_update(void);
#define get_button(e) ((button_t*)get_component(&Button, e))
#define add_button(e) ((button_t*)add_component(&Button, e))

#ifdef __cplusplus
}
#endif

#endif // SCENEX_BUTTON_H
