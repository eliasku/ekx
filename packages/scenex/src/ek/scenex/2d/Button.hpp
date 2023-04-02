#pragma once

#include <ek/util/Signal.hpp>
#include <ek/math.h>
#include "MovieClip.hpp"
#include <ekx/app/time_layers.h>

#define BUTTON_EVENT_CLICK H("button_click")

namespace ek {

struct ButtonSkin {
    string_hash_t sfx_over = H("sfx/btn_over");
    string_hash_t sfx_down = H("sfx/btn_down");
    string_hash_t sfx_click = H("sfx/btn_click");
    string_hash_t sfx_out = H("sfx/btn_out");
    string_hash_t sfx_cancel = H("sfx/btn_cancel");

    float over_speed_forward = 8.0f;
    float over_speed_backward = 8.0f;
    float push_speed_forward = 8.0f;
    float push_speed_backward = 8.0f;
};

struct Button {
    ButtonSkin* skin = nullptr;

    float time_over = 0.0f;
    float time_push = 0.0f;
    float time_post = 0.0f;

    vec2_t base_skew = vec2(0, 0);
    vec2_t base_scale = vec2(1,1);
    color2_t base_color = color2_identity();

    TimeLayer time = TIME_LAYER_UI;

    bool initialized = false;
//    bool back_button = false;
    bool pushed = false;

    static void updateAll();
};

}


