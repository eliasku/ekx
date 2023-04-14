#ifndef SCENEX_PARTICLE_H
#define SCENEX_PARTICLE_H

#include "particle_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    R(sprite_t) sprite;
    //int draw_layer = 0;
    //const char* text = nullptr;
    vec2_t pivot;

    float time;
    float time_total;
    vec2_t position;
    vec2_t velocity;
    vec2_t acc;

    float acc_x_phase;
    float acc_x_speed;

    // angle state
    float angle_base;

    // rotating and rotation speed
    float rotation;
    float rotation_speed;
    float angle_velocity_factor;

    // initial alpha
    particle_alpha_mode_t alpha_mode;
    float alpha;
    color_t color;
    color_t offset;

    particle_scale_mode_t scale_mode;
    float scale_off_time;
    float scale_start;
    float scale_end;

    // current state
    vec2_t scale;
    bool reflector;
    rect_t bounds;
} particle_t;

particle_t particle(void);
void update_particle(particle_t* p, float dt);
rect_t get_particle_bounds(particle_t* p);
void draw_particle(particle_t* p);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_PARTICLE_H
