#include "particle.h"
#include <ek/canvas.h>

particle_t particle(void) {
    particle_t r = INIT_ZERO;
    r.acc_x_phase = MATH_PI / 2.0f;
    r.alpha = 1.0f;
    r.color = COLOR_WHITE;
    r.scale_start = 1.0f;
    r.scale = vec2(1,1);
    r.bounds = rect_01();
    return r;
}


static void apply_particle_values(particle_t* p) {
    switch (p->scale_mode) {
        case PARTICLE_SCALE_COS_OUT: {
            float s = cosf(0.5f * MATH_PI * (1.0f - p->time / p->time_total));
            p->scale = vec2(s, s);
        }
            break;
        case PARTICLE_SCALE_RANGE: {
            float time_max = p->scale_off_time > 0.0f ? p->scale_off_time : p->time_total;
            float ratio = 1.0f - saturate(p->time / time_max);
            float s = lerp_f32(p->scale_start, p->scale_end, ratio);
            p->scale = vec2(s, s);
        }
            break;
        default:
            p->scale = vec2(p->scale_start, p->scale_start);
            break;
    }

    switch (p->alpha_mode) {
        case PARTICLE_ALPHA_BY_SCALE:
            p->color.a = unorm8_f32_clamped(p->alpha * length_vec2(p->scale));
            break;
        case PARTICLE_ALPHA_LIFE_SIN:
            p->color.a = unorm8_f32_clamped(p->alpha * sinf(MATH_PI * p->time / p->time_total));
            break;
        case PARTICLE_ALPHA_DC_BLINK: {
            float a = 0.25f;
            if (p->time > 0.75f) {
                a = 1.0f - p->time;
            } else if (p->time < 0.25f) {
                a = p->time;
            }
            a *= 4.0f;
            p->color.a = unorm8_f32_clamped(p->alpha * a);
        }
            break;
        case PARTICLE_ALPHA_QUAD_OUT: {
            float x = 1.0f - p->time / p->time_total;
            p->color.a = unorm8_f32_clamped(p->alpha * (1.0f - x * x));
        }
            break;
        default:
            p->color.a = unorm8_f32_clamped(p->alpha);
            break;
    }
}

void update_particle(particle_t* p, float dt) {
    const float ax = p->acc.x * sinf(p->acc_x_phase + p->time * p->acc_x_speed);
    p->velocity.x += ax * dt;
    p->velocity.y += p->acc.y * dt;

    p->position.x += p->velocity.x * dt;
    p->position.y += p->velocity.y * dt;

    if (p->reflector && p->position.y + 4.0f > 0.0f) {
        p->position.y = -4.0f;
        p->velocity.x = 0.7f * p->velocity.x;
        p->velocity.y = -0.5f * p->velocity.y;
    }

    p->rotation += p->rotation_speed * dt;

    apply_particle_values(p);

    p->time -= dt;
}

rect_t get_particle_bounds(particle_t* p) {
    if (p->sprite) {
        const sprite_t* spr = &REF_RESOLVE(res_sprite, p->sprite);
        p->bounds = spr->rect;
    }
    return p->bounds;
}

void draw_particle(particle_t* p) {
    const float vis_angle = p->angle_base + p->rotation + p->angle_velocity_factor * atan2f(p->velocity.y, p->velocity.x);
    canvas_save_transform();
    canvas_transform_pivot(p->position, vis_angle, p->scale, p->pivot);
    canvas_concat_color((color2_t){p->color, p->offset});
    if (p->sprite) {
        const sprite_t* spr = &REF_RESOLVE(res_sprite, p->sprite);
        draw_sprite(spr);
    }
    canvas_restore_transform();
}
