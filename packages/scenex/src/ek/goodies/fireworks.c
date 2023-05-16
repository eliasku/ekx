#include "fireworks.h"

#include <ek/buf.h>
#include <ek/scenex/2d/layout_rect.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/particles/particle_system.h>
#include <ekx/app/audio_manager.h>
#include <ekx/app/time_layers.h>

struct {
    entity_t layer;
    time_layer_t time_layer;
    float timer_;
    bool enabled;
} fireworks;

void start_fireworks(entity_t e) {
    fireworks.layer = e;
    fireworks.enabled = true;

    add_particle_layer2d(e);
    particle_renderer2d_setup(e)->target = e;
    particle_emitter2d_t* emitter = add_particle_emitter2d(e);
    emitter->data.burst = 0;
    emitter->particle = R_PARTICLE(H("firework_star"));
    emitter->layer = e;
    emitter->enabled = true;
    set_touchable(e, false);
}

void update_fireworks(void) {
    entity_t e = fireworks.layer;
    if (!is_entity(e) || !fireworks.enabled) {
        return;
    }
    fireworks.timer_ -= g_time_layers[fireworks.time_layer].dt;
    if (fireworks.timer_ <= 0) {
        particle_emitter2d_t* emitter = get_particle_emitter2d(e);

        rect_t rect = find_parent_layout_rect(e, true);
        rect.h *= 0.5f;

        emitter->position = add_vec2(rect.position, mul_vec2(rect.size, vec2(random_f(), random_f())));
        play_sound(H("sfx/firework"), random_range_f(0.5f, 1.0f), 1);
        particle_decl_t* part = &RES_NAME_RESOLVE(res_particle, H("firework_star"));
        switch (random_n(4)) {
            case 0:
                part->color = rand_color_gradient(ARGB(0xffffff00), ARGB(0xffff7f00));
                break;
            case 1:
                part->color = rand_color_gradient(ARGB(0xfffea7f9), ARGB(0xffff006c));
                break;
            case 2:
                part->color = rand_color_gradient(ARGB(0xffd5fdfd), ARGB(0xff00c0ff));
                break;
            case 3:
                part->color = rand_color_gradient(ARGB(0xff2e0678), ARGB(0xffb066cf));
                break;
            default:
                break;
        }

        emitter->particle = R_PARTICLE(H("firework_spark"));
        emitter->data.acc = float_range(0, 100);
        emitter->data.speed = float_range(50, 100);
        particles_burst(e, random_range_i(20, 30), vec2(0, 0));

        emitter->particle = R_PARTICLE(H("firework_star"));
        emitter->data.speed = float_range(10, 100);
        emitter->data.acc = float_range(0, -50);
        particles_burst(e, random_range_i(60, 80), vec2(0, 0));

        fireworks.timer_ = random_range_f(0.1f, 1.0f);
        //            _timer = random_range_f(0.01f, 0.1f);
    }
}

void stop_fireworks(void) {
    entity_t e = fireworks.layer;
    if (is_entity(e)) {
        particle_layer2d_t* layer = get_particle_layer2d(e);
        if (layer) {
            arr_clear(layer->particles);
        }
    }
    fireworks.enabled = false;
    fireworks.layer = NULL_ENTITY;
    fireworks.timer_ = 0.0f;
}
