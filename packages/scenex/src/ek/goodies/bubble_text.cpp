#include "bubble_text.h"

#include <ek/scenex/base/destroy_timer.h>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/node_api.h>
#include <ek/scenex/scene_factory.h>

#include <ek/rnd.h>
#include <ekx/app/time_layers.h>
#include <ek/math.h>

inline float ease_back5(float t) {
    return ease_back(t, 5);
}

void bubble_text_update(void) {
    float dt = g_time_layers[TIME_LAYER_HUD].dt;
    const float time_max = 2.0f;
    const float delta_y = -100.0f;

    for (auto e: ecs::view<bubble_text_t>()) {
        auto& state = ecs::get<bubble_text_t>(e);

        if (state.delay > 0.0f) {
            state.delay -= dt;
            continue;
        }

        state.time += dt;
        float r = saturate(state.time / time_max);
        float sc = 1.0f;
        float sct;
        vec2_t off = state.offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = ease_out(ease_p3_out(sct), ease_back5);
            off = state.offset * ease_p3_out(sct);
        }

        auto& transform = ecs::get<transform2d_t>(e);
        transform.set_scale(sc);
        vec2_t fly_pos = vec2(0, delta_y * ease_p3_out(r));
        transform.set_position(state.start + off + fly_pos);
        transform.color.scale.a = unorm8_f32_clamped(1.0f - (r * r * r));
        transform.color.offset.a = unorm8_f32_clamped(r * r * r);

        if (state.time >= time_max) {
            destroy_later(e, 0, 0);
        }
    }
}

entity_t bubble_text_entity(string_hash_t fontName, vec2_t pos, float delay) {
    const float spread = 10.0f;
    const float font_size = 32.0f;

    entity_t e = create_node2d(0);
    bubble_text_t& c = ecs::add<bubble_text_t>(e);
    c.delay = delay;
    c.time = 0.0f;
    c.start = pos;
    c.offset = vec2(random_range_f(-spread, spread), 0.0f);
    ek::TextFormat format{fontName, font_size};
    format.setAlignment(ALIGNMENT_CENTER);
    format.setTextColor(COLOR_WHITE);
    format.addShadow(COLOR_BLACK, 4, vec2(0.0f, 1.5f));

    text2d_setup_ex(e, format);

    set_touchable(e, false);
    set_scale(e, vec2(0, 0));
    return e;
}

/*
 private function checkOversteps():void {

        var displayWidth:int = Seks.display.width;
		var maxScale:Number = 1.5;
		var halfWidth:int = textField.width * 0.5 * maxScale;
		var global:Point = _producer.layer.localToGlobal(new Point(_startX + _offsetX, _startY));
        var leftOverstep:int = global.x - halfWidth;
        var rightOverstep:int = global.x + halfWidth;

        if (leftOverstep < 0) {
           _startX += Math.abs(leftOverstep);
        }

        if (rightOverstep > displayWidth) {
            _startX -= Math.abs(displayWidth - rightOverstep);
        }
    }
 */
