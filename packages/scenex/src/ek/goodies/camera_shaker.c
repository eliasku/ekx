#include "camera_shaker.h"

#include <ek/rnd.h>
#include <ek/scenex/2d/transform2d.h>

ECX_DEFINE_TYPE(camera_shaker_t);

void setup_camera_shaker(void) {
    ECX_TYPE(camera_shaker_t, 8);
}

void camera_shaker_start(entity_t e, float v) {
    EK_ASSERT(ECX_ID(camera_shaker_t).index);
    camera_shaker_t* c = (camera_shaker_t*) get_component(&ECX_ID(camera_shaker_t), e);
    if (c) {
        c->state = MAX(v, c->state);
    }
}

void camera_shaker_set(entity_t e) {
    EK_ASSERT(ECX_ID(camera_shaker_t).index);
    camera_shaker_t* c = (camera_shaker_t*) add_component(&ECX_ID(camera_shaker_t), e);
    *c = (camera_shaker_t) {
            TIME_LAYER_ROOT,
            0.0f,
            0.25f,
            vec2(8.0f, 8.0f),
            vec2(0.25f, 0.25f),
    };
}

static
vec2_t random_f2(float min, float max) {
    return vec2(random_range_f(min, max), random_range_f(min, max));
}

void update_camera_shakers(void) {
    EK_ASSERT(ECX_ID(camera_shaker_t).index);
    for (uint32_t i = 1; i < ECX_ID(camera_shaker_t).size; ++i) {
        entity_idx_t ei = ECX_ID(camera_shaker_t).handle_to_entity[i];
        entity_t e = entity_at(ei);
        camera_shaker_t* s = ((camera_shaker_t*) ECX_ID(camera_shaker_t).data[0]) + i;
        const float dt = g_time_layers[s->timer].dt;
        s->state = reach(s->state, 0.0f, dt);
        const float r = integrate_exp(0.9f, dt, 0);

        const vec2_t pos1 = scale_vec2(mul_vec2(s->offset, random_f2(0.0f, 1.0f)), s->state);
        const vec2_t scale1 = add_vec2(vec2(1, 1), scale_vec2(mul_vec2(s->scale, random_f2(-0.5f, 0.5f)), s->state));
        const float rot1 = random_range_f(-0.5f, 0.5f) * s->rotation * s->state;
        set_position(e, lerp_vec2(get_position(e), pos1, r));
        set_scale(e, lerp_vec2(get_scale(e), scale1, r));
        set_rotation(e, lerp_f32(get_rotation(e), rot1, r));
    }
}
