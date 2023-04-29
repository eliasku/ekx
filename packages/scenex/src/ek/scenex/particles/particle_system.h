#ifndef SCENEX_PARTICLE_SYSTEM_H
#define SCENEX_PARTICLE_SYSTEM_H

#include "particle_decl.h"
#include "particle.h"
#include <ecx/ecx.h>
#include <ekx/app/time_layers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    particle_t* particles;
    time_layer_t timer;
    bool keep_alive;
} particle_layer2d_t;

typedef struct {
    emitter_data_t data;
    vec2_t position;
    vec2_t velocity;

    void (* on_spawn)(entity_t, particle_t*);

    R(ParticleDecl) particle;
    entity_t layer;
    float time;
    time_layer_t timer;
    bool enabled;
} particle_emitter2d_t;

typedef struct {
    entity_t target;
} particle_renderer2d_t;

particle_renderer2d_t* particle_renderer2d_setup(entity_t e);

void particles_burst(entity_t e, int count, vec2_t relative_velocity);

particle_t* spawn_particle(entity_t e, string_hash_t particle_id);

void spawnFromEmitter(entity_t src, entity_t to_layer, const particle_decl_t* decl, particle_emitter2d_t* emitter,
                      int count);

void update_emitters(void);

void update_particles(void);

void draw_particle_layer(entity_t e);

particle_layer2d_t* find_particle_layer(entity_t e);

particle_t* produce_particle(particle_layer2d_t* to_layer, const particle_decl_t* decl);

#define ParticleEmitter2D ECX_ID(particle_emitter2d_t)
#define ParticleLayer2D ECX_ID(particle_layer2d_t)
#define ParticleRenderer2D ECX_ID(particle_renderer2d_t)

extern ECX_DEFINE_TYPE(particle_emitter2d_t);
extern ECX_DEFINE_TYPE(particle_layer2d_t);
extern ECX_DEFINE_TYPE(particle_renderer2d_t);

void setup_particle2d(void);

#define get_particle_emitter2d(e) ECX_GET(particle_emitter2d_t,e)
#define add_particle_emitter2d(e) ECX_ADD(particle_emitter2d_t,e)
#define get_particle_layer2d(e) ECX_GET(particle_layer2d_t,e)
#define add_particle_layer2d(e) ECX_ADD(particle_layer2d_t,e)
#define get_particle_renderer2d(e) ECX_GET(particle_renderer2d_t,e)
#define add_particle_renderer2d(e) ECX_ADD(particle_renderer2d_t,e)

#ifdef __cplusplus
}
#endif

#endif // SCENEX_PARTICLE_SYSTEM_H
