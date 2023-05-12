#include "button.h"

#include <ek/math.h>
#include <ek/rnd.h>

#include <ekx/app/audio_manager.h>

#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/base/node_events.h>

button_skin_t button_skin_default;

ECX_DEFINE_TYPE(button_t);
#define Button ECX_ID(button_t)

static void button_ctor(component_handle_t handle) {
    ((button_t*)Button.data[0])[handle] = button();
}

void setup_button(void) {
    ECX_TYPE(button_t, 16);
    Button.ctor = button_ctor;

    button_skin_default = (button_skin_t){
        .sfx_over = H("sfx/btn_over"),
        .sfx_down = H("sfx/btn_down"),
        .sfx_click = H("sfx/btn_click"),
        .sfx_out = H("sfx/btn_out"),
        .sfx_cancel = H("sfx/btn_cancel"),

        .over_speed_forward = 8.0f,
        .over_speed_backward = 8.0f,
        .push_speed_forward = 8.0f,
        .push_speed_backward = 8.0f,
    };
}

button_t button(void) {
    return (button_t){
        .skin = &button_skin_default,
        .time_over = 0.0f,
        .time_push = 0.0f,
        .time_post = 0.0f,
        .base_skew = vec2(0, 0),
        .base_scale = vec2(1, 1),
        .base_color = color2_identity(),
        .time = TIME_LAYER_UI,
        .initialized = false,
        .pushed = false,
    };
}

static inline void play_button_sound(string_hash_t id) {
    play_sound(id, 1, 1);
}

static void start_post_tween(button_t* btn) {
    btn->time_post = fmaxf(random_range_f(0.7f, 1.0f), btn->time_post);
}

static void initialize_base_transform(button_t* btn, const transform2d_t* transform) {
    btn->base_color = transform->color;
    btn->base_scale = transform->cached_scale;
    btn->base_skew = transform->cached_skew;
}

static void apply_skin(const button_skin_t* skin, const button_t* btn, transform2d_t* transform) {
    // TODO: skin params?
    (void)skin;

    const float over = btn->time_over;
    const float push = btn->time_push;
    const float post = btn->time_post;
    const float pi = MATH_PI;

    const float sx = 1.0f + 0.2f * sinf((1.0f - post) * pi * 5.0f) * post;
    const float sy = 1.0f + 0.2f * sinf((1.0f - post) * pi) * cosf((1.0f - post) * pi * 5.0f) * post;

    transform2d_set_scale(transform, mul_vec2(btn->base_scale, vec2(sx, sy)));

    const color_t color = lerp_color(COLOR_WHITE, ARGB(0xFF888888), push);
    transform->color.scale = mul_color(btn->base_color.scale, color);

    const float h = 0.1f * over;
    transform->color.offset = add_color(btn->base_color.offset, color_4f(h, h, h, 0));
}

static void update_movie_frame(entity_t entity, const interactive_t* interactive) {
    movieclip_t* mc = get_movieclip(entity);
    if (mc) {
        int frame = 0;
        if (interactive->over || interactive->pushed) {
            frame = 1;
            if (interactive->pushed && interactive->over) {
                frame = 2;
            }
        }
        goto_and_stop(entity, (float)frame);
    }
}

void update_button_events(interactive_t* interactive, button_t* button, entity_t e, entity_t* queue, uint32_t* queue_num) {
    if (interactive->ev_over) {
        play_button_sound(button->skin->sfx_over);
    }
    if (interactive->ev_out) {
        if (button->pushed) {
            start_post_tween(button);
            play_button_sound(button->skin->sfx_cancel);
        } else {
            play_button_sound(button->skin->sfx_out);
        }
        button->pushed = false;
    }
    if (interactive->ev_down) {
        play_button_sound(button->skin->sfx_down);
    }
    if (interactive->ev_tap) {
        if (get_node_events(e)) {
            queue[*queue_num] = e;
            ++(*queue_num);
        }
        play_button_sound(button->skin->sfx_click);
        start_post_tween(button);
    }
    button->pushed = interactive->pushed;
}

void update_buttons(void) {
    entity_t tap_events[64];
    uint32_t tap_events_num = 0;
    for (uint32_t i = 1; i < Button.size; ++i) {
        const entity_t e = get_entity(&Button, i);
        button_t* btn = (button_t*)Button.data[0] + i;
        interactive_t* interactive = get_interactive(e);
        transform2d_t* transform = get_transform2d(e);
        if (interactive && transform) {
            float dt = g_time_layers[btn->time].dt;

            if (!btn->initialized) {
                btn->initialized = true;

                initialize_base_transform(btn, transform);
                //initialize_events(e);
            }

            update_button_events(interactive, btn, e, tap_events, &tap_events_num);

            const button_skin_t* skin = btn->skin;

            btn->time_over = reach_delta(btn->time_over,
                                         interactive->over ? 1.0f : 0.0f,
                                         dt * skin->over_speed_forward,
                                         -dt * skin->over_speed_backward);

            btn->time_push = reach_delta(btn->time_push,
                                         interactive->pushed ? 1.0f : 0.0f,
                                         dt * skin->push_speed_forward,
                                         -dt * skin->push_speed_backward);

            btn->time_post = reach(btn->time_post, 0.0f, 2.0f * dt);

            apply_skin(skin, btn, transform);
            update_movie_frame(e, interactive);
        }
    }

    for (uint32_t i = 0; i < tap_events_num; ++i) {
        const entity_t e = tap_events[i];
        if (is_entity(e)) {
            const node_event_t event = node_event(BUTTON_EVENT_CLICK, e);
            emit_node_event(e, &event);
        }
    }
}
