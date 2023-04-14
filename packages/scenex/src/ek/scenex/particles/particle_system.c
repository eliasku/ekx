#include "particle_system.h"

#include <ek/buf.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/2d/transform2d.h>

ecx_component_type ParticleEmitter2D;
ecx_component_type ParticleLayer2D;
ecx_component_type ParticleRenderer2D;

void ParticleEmitter2D_ctor(component_handle_t i) {
    particle_emitter2d_t r = INIT_ZERO;
    r.data = emitter_data();
    r.enabled = true;
    ((particle_emitter2d_t*) ParticleEmitter2D.data[0])[i] = r;
}

void ParticleLayer2D_ctor(component_handle_t i) {
    ((particle_layer2d_t*) ParticleLayer2D.data[0])[i] = (particle_layer2d_t) {0};
}

void ParticleLayer2D_dtor(component_handle_t i) {
    particle_layer2d_t* l = ((particle_layer2d_t*) ParticleLayer2D.data[0]) + i;
    arr_reset((void**) &l->particles);
}

void setup_particle2d(void) {
    init_component_type(&ParticleEmitter2D, (ecx_component_type_decl) {
            "ParticleEmitter2D", 8, 1, {sizeof(particle_emitter2d_t)}
    });
    ParticleEmitter2D.ctor = ParticleEmitter2D_ctor;

    init_component_type(&ParticleLayer2D, (ecx_component_type_decl) {
            "ParticleLayer2D", 8, 1, {sizeof(particle_layer2d_t)}
    });
    ParticleLayer2D.ctor = ParticleLayer2D_ctor;
    ParticleLayer2D.dtor = ParticleLayer2D_dtor;

    init_component_type(&ParticleRenderer2D, (ecx_component_type_decl) {
            "ParticleRenderer2D", 8, 1, {sizeof(particle_renderer2d_t)}
    });
}

particle_layer2d_t* find_particle_layer(entity_t e) {
    entity_t l = ((particle_emitter2d_t*) get_component_or_default(&ParticleEmitter2D, e))->layer;
    if (!is_entity(l)) {
        l = e;
    }
    return (particle_layer2d_t*) add_component(&ParticleLayer2D, l);
}

particle_t* produce_particle(particle_layer2d_t* to_layer, const particle_decl_t* decl) {
    particle_t p = INIT_ZERO;
    p.sprite = decl->sprite;
    p.reflector = decl->use_reflector;
    p.acc = decl->acceleration;
    p.alpha_mode = decl->alpha_mode;
    p.scale_mode = decl->scale_mode;
    p.acc_x_phase = random_float_range(decl->acc_x_phase);
    p.acc_x_speed = random_float_range(decl->acc_x_speed);
    p.scale_off_time = decl->scale_off_time;
    p.scale_start = random_float_range(decl->scale_start);
    p.scale_end = random_float_range(decl->scale_end);
    p.color = rand_color_next((rand_color_t*) &decl->color);
    p.angle_velocity_factor = decl->angle_velocity_factor;
    p.angle_base = decl->angle_base;
    p.rotation_speed = decl->rotation_speed;
    p.rotation = random_float_range(decl->rotation);
    p.alpha = random_float_range(decl->alpha_start);
    p.offset = decl->color_offset;
    p.offset.a = unorm8_f32_clamped(decl->additive);
    p.time_total = p.time = random_float_range(decl->life_time);
    arr_push(to_layer->particles, p);
    return arr_back(to_layer->particles);
}

void particles_burst(entity_t e, int count, vec2_t relative_velocity) {
    if (count < 0) {
        return;
    }
    const particle_emitter2d_t* emitter = get_particle_emitter2d(e);
    const emitter_data_t* data = &emitter->data;
    const entity_t layer_entity = emitter->layer;
    particle_layer2d_t* layer = get_particle_layer2d(layer_entity);
    const vec2_t position = local_to_local(e, layer_entity, emitter->position);
    float a = random_float_range(data->dir);
    particle_decl_t* decl = &REF_RESOLVE(res_particle, emitter->particle);
    while (count > 0) {
        particle_t* p = produce_particle(layer, decl);
        vec2_t pos = position;
        pos.x += random_range_f(data->rect.x, RECT_R(data->rect));
        pos.y += random_range_f(data->rect.y, RECT_B(data->rect));
        p->position = pos;
        const float speed = random_float_range(data->speed);
        const float acc = random_float_range(data->acc);
        const vec2_t dir = vec2_cs(a);
        p->velocity = add_vec2(scale_vec2(dir, speed), relative_velocity);
        p->acc = add_vec2(p->acc, scale_vec2(dir, acc));
        if (emitter->on_spawn) {
            emitter->on_spawn(e, p);
        }
        --count;
        a += random_float_range(data->burst_rotation_delta);
    }
}

void update_emitters(void) {
    for (uint32_t i = 1; i < ParticleEmitter2D.size; ++i) {
        const entity_t e = get_entity(&ParticleEmitter2D, i);
        particle_emitter2d_t* emitter = (particle_emitter2d_t*) get_component_data(&ParticleEmitter2D, i, 0);
        entity_t emitter_layer = emitter->layer;
        if (!emitter->enabled || !emitter->particle || !is_entity(emitter_layer)) {
            continue;
        }
        const float dt = g_time_layers[emitter->timer].dt;
        emitter->time += dt;
        const emitter_data_t* data = &emitter->data;
        if (emitter->time >= data->interval) {
            particle_layer2d_t* layer = get_particle_layer2d(emitter_layer);
            vec2_t position = data->offset;
            position = local_to_local(e, emitter_layer, position);

            particle_decl_t* decl = &REF_RESOLVE(res_particle, emitter->particle);
            int count = data->burst;
            float a = random_float_range(data->dir);
            while (count > 0) {
                particle_t* p = produce_particle(layer, decl);
                vec2_t pos = add_vec2(position, data->offset);
                pos.x += random_range_f(data->rect.x, RECT_R(data->rect));
                pos.y += random_range_f(data->rect.y, RECT_B(data->rect));
                p->position = pos;
                float speed = random_float_range(data->speed);
                float acc = random_float_range(data->acc);
                const vec2_t dir = vec2_cs(a);
                p->velocity = scale_vec2(dir, speed);
                p->acc = scale_vec2(dir, acc);

                if (emitter->on_spawn) {
                    emitter->on_spawn(e, p);
                }

                --count;
                a += random_float_range(data->burst_rotation_delta);
            }
            emitter->time = 0.0f;
        }
    }
}

particle_t* spawn_particle(entity_t e, string_hash_t particle_id) {
    particle_decl_t* decl = &RES_NAME_RESOLVE(res_particle, particle_id);
    particle_layer2d_t* to_layer = add_particle_layer2d(e);
    return produce_particle(to_layer, decl);
}

void spawnFromEmitter(entity_t src, entity_t to_layer, const particle_decl_t* decl, particle_emitter2d_t* emitter,
                      int count) {
    if (count <= 0) {
        return;
    }
    const emitter_data_t* data = &emitter->data;
    float a = random_float_range(data->dir);
    particle_layer2d_t* layer_comp = get_particle_layer2d(to_layer);
    while (count > 0) {
        particle_t* p = produce_particle(layer_comp, decl);
        const vec2_t position = add_vec2(
                data->offset,
                add_vec2(
                        data->rect.position,
                        mul_vec2(
                                data->rect.size,
                                vec2(random_f(), random_f())
                        )
                )
        );
        const vec2_t dir = vec2_cs(a);
        const float speed = random_float_range(data->speed);
        const float acc = random_float_range(data->acc);
        p->position = local_to_local(src, to_layer, position);
        p->velocity = add_vec2(scale_vec2(dir, speed), emitter->velocity);
        p->acc = add_vec2(p->acc, scale_vec2(dir, acc));
        if (emitter->on_spawn) {
            // TODO: EMITTER ENTITY!? and not `src` entity or `layer` entity?
            emitter->on_spawn(src, p);
        }

        --count;
        a += random_float_range(data->burst_rotation_delta);
    }
}

void update_particles(void) {
    for (uint32_t i = 1; i < ParticleLayer2D.size; ++i) {
        particle_layer2d_t* layer = (particle_layer2d_t*) get_component_data(&ParticleLayer2D, i, 0);
        const float dt = g_time_layers[layer->timer].dt;
        particle_t* particles = layer->particles;
        uint32_t j = 0;
        while (j < arr_size(particles)) {
            particle_t* p = particles + j;
            update_particle(p, dt);
            if (p->time > 0) {
                ++j;
            } else {
                arr_swap_remove_(particles, sizeof(particle_t), j);
            }
        }
    }
}

static void particle_renderer2d_draw(entity_t e) {
    entity_t target_entity = get_particle_renderer2d(e)->target;
    if (is_entity(target_entity)) {
        particle_layer2d_t* layer = get_particle_layer2d(target_entity);
        if (layer) {
            arr_for(p, layer->particles) {
                draw_particle(p);
            }
        }
    }
}

particle_renderer2d_t* particle_renderer2d_setup(entity_t e) {
    display2d_t* display = add_display2d(e);
    particle_renderer2d_t* pr = add_particle_renderer2d(e);
    pr->target = e;
    display->draw = particle_renderer2d_draw;
    return pr;
}
