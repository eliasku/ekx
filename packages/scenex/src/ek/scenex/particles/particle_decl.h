#ifndef SCENEX_PARTICLE_DECL_H
#define SCENEX_PARTICLE_DECL_H

#include <sce/sprite.h>
#include <ek/rnd.h>
#include <ek/rr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PARTICLE_SCALE_NONE = 0,
    PARTICLE_SCALE_COS_OUT = 1,
    PARTICLE_SCALE_RANGE = 2,
} particle_scale_mode_t;

typedef enum {
    PARTICLE_ALPHA_NONE = 0,
    PARTICLE_ALPHA_BY_SCALE = 1,
    PARTICLE_ALPHA_LIFE_SIN = 2,
    PARTICLE_ALPHA_DC_BLINK = 3,
    PARTICLE_ALPHA_QUAD_OUT = 4,
} particle_alpha_mode_t;

typedef enum {
    RAND_COLOR_CONTINUOUS = 0,
    RAND_COLOR_RAND_LERP = 1,
    RAND_COLOR_RAND_ELEMENT = 2,
} rand_color_mode_t;

typedef struct {
    color_t colors[8];
    uint32_t colors_num;
    rand_color_mode_t mode;
    uint32_t state;
} rand_color_t;

rand_color_t rand_color(color_t color);

rand_color_t rand_color_gradient(color_t color1, color_t color2);

rand_color_t rand_color_steps(color_t color1, color_t color2);

color_t rand_color_next(rand_color_t* color);

typedef struct {
    float min;
    float max;
} float_range_t;

float_range_t float_range_zero(void);

float_range_t float_range_val(float val);

float_range_t float_range(float min, float max);

float random_float_range(float_range_t range);

typedef struct {
    R(sprite_t) sprite;
    particle_scale_mode_t scale_mode;
    particle_alpha_mode_t alpha_mode;
    float_range_t alpha_start;
    vec2_t acceleration;
    bool use_reflector;

    float_range_t life_time;
    float_range_t acc_x_phase;
    float_range_t acc_x_speed;

    float scale_off_time;
    float_range_t scale_start;
    float_range_t scale_end;

    rand_color_t color;
    color_t color_offset;
    float additive;

    float_range_t rotation;
    float rotation_speed;
    float angle_velocity_factor;
    float angle_base;
} particle_decl_t;

particle_decl_t
particle_decl(void);

struct res_particle {
    string_hash_t names[32];
    particle_decl_t data[32];
    rr_man_t rr;
};

#define R_PARTICLE(name) REF_NAME(res_particle, name)

extern struct res_particle res_particle;

void setup_res_particle(void);

typedef struct {
    float interval;
    int burst;
    rect_t rect;
    vec2_t offset;
    float_range_t burst_rotation_delta;
    float_range_t speed;
    float_range_t acc;
    float_range_t dir;
} emitter_data_t;

emitter_data_t emitter_data(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_PARTICLE_DECL_H
