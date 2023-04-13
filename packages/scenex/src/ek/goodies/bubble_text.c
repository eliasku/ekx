#include "bubble_text.h"

#include <ek/scenex/base/node.h>

#include <ek/scenex/base/destroy_timer.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/scene_factory.h>
#include <ek/scenex/text/text_format.h>
#include <ek/scenex/2d/text2d.h>

#include <ek/rnd.h>
#include <ekx/app/time_layers.h>
#include <ek/math.h>

ecx_component_type BubbleText;
void bubble_text_setup(void) {
    init_component_type(&BubbleText, (ecx_component_type_decl) {
            "BubbleText", 8, 1, {sizeof(bubble_text_t)}
    });
}

inline static float ease_back5(float t) {
    return ease_back(t, 5);
}

void bubble_text_update(void) {
    float dt = g_time_layers[TIME_LAYER_HUD].dt;
    const float time_max = 2.0f;
    const float delta_y = -100.0f;

    for (uint32_t i = 1; i < BubbleText.size; ++i) {
        bubble_text_t* state = (bubble_text_t*)get_component_data(&BubbleText, i, 0);
        const entity_t e = get_entity(&BubbleText, i);

        if (state->delay > 0.0f) {
            state->delay -= dt;
            continue;
        }

        state->time += dt;
        float r = saturate(state->time / time_max);
        float sc = 1.0f;
        float sct;
        vec2_t off = state->offset;
        if (r < 0.5f) {
            sct = r * 2.0f;
            sc = ease_out(ease_p3_out(sct), ease_back5);
            off = scale_vec2(state->offset, ease_p3_out(sct));
        }

        transform2d_t* transform = get_transform2d(e);
        transform2d_set_scale_f(transform, sc);
        vec2_t fly_pos = vec2(0, delta_y * ease_p3_out(r));
        transform->pos = add_vec2(state->start, add_vec2(off, fly_pos));
        transform->color.scale.a = unorm8_f32_clamped(1.0f - (r * r * r));
        transform->color.offset.a = unorm8_f32_clamped(r * r * r);

        if (state->time >= time_max) {
            destroy_later(e, 0, 0);
        }
    }
}

entity_t bubble_text_entity(string_hash_t font_name, vec2_t pos, float delay) {
    const float spread = 10.0f;
    const float font_size = 32.0f;

    entity_t e = create_node2d(0);
    bubble_text_t* c = (bubble_text_t*)add_component(&BubbleText, e);
    c->delay = delay;
    c->time = 0.0f;
    c->start = pos;
    c->offset = vec2(random_range_f(-spread, spread), 0.0f);
    text_format_t format = text_format(font_name, font_size);
    text_format_align(&format, ALIGNMENT_CENTER);
    text_format_set_text_color(&format, COLOR_WHITE);
    text_format_add_shadow(&format, COLOR_BLACK, 4, vec2(0.0f, 1.5f), 0.2f);
    
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
