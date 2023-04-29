#include "simple_animator.h"

#include <ek/scenex/2d/transform2d.h>
#include <ek/math.h>

ECX_DEFINE_TYPE(simple_animator_t);
#define SimpleAnimator ECX_ID(simple_animator_t)

void setup_simple_animator(void) {
    ECX_TYPE(simple_animator_t, 4);
}

void update_simple_animators(float dt) {
    EK_ASSERT(SimpleAnimator.index);
    for (uint32_t i = 1; i < SimpleAnimator.size; ++i) {
        entity_idx_t ei = SimpleAnimator.handle_to_entity[i];
        entity_t e = entity_at(ei);
        simple_animator_t* s = ((simple_animator_t*) SimpleAnimator.data[0]) + i;
        s->rotation += dt * s->rotation_speed;
        s->hue += dt * s->hue_speed;
        set_rotation(e, s->rotation);
        set_color(e, lerp_color(
                color_hue(reduce(s->hue, 1.0f, 0.0f)),
                s->base_color,
                1.0f - s->hue_mixup_factor
        ));
    }
}

simple_animator_t* add_simple_animator(entity_t e) {
    EK_ASSERT(SimpleAnimator.index);
    simple_animator_t* s = (simple_animator_t*) add_component(&SimpleAnimator, e);
    *s = (simple_animator_t) {
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            COLOR_WHITE,
    };
    return s;
}
