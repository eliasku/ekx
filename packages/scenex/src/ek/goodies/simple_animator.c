#include "simple_animator.h"

#include <ek/scenex/2d/transform2d.h>
#include <ek/math.h>

ecx_component_type simple_animator_comp;

void simple_animator_init(void) {
    init_component_type(&simple_animator_comp, (ecx_component_type_decl) {
            "simple_animator", 4, 1, {sizeof(simple_animator_t)}
    });
}

void simple_animator_update(float dt) {
    EK_ASSERT(simple_animator_comp.index);
    for (uint32_t i = 1; i < simple_animator_comp.size; ++i) {
        entity_idx_t ei = simple_animator_comp.handle_to_entity[i];
        entity_t e = entity_at(ei);
        simple_animator_t* s = ((simple_animator_t*) simple_animator_comp.data[0]) + i;
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

simple_animator_t* simple_animator_add(entity_t e) {
    EK_ASSERT(simple_animator_comp.index);
    simple_animator_t* s = (simple_animator_t*) add_component(&simple_animator_comp, e);
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
