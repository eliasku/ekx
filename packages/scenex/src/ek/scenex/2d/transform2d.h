#ifndef SCENEX_TRANSFORM2D_H
#define SCENEX_TRANSFORM2D_H

#include <ecx/ecx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

// 32 bytes
typedef struct {
    mat3x2_t matrix; // 4 * 6 = 24
    color2_t color; // 2 * 4 = 8
} world_transform2d_t;

// TODO: mat2x2, position,
typedef struct {
    union {
        mat3x2_t matrix;
        struct {
            mat2_t rot;
            union {
                vec2_t pos;
                struct {
                    float x;
                    float y;
                };
            };
        };
    };
    color2_t color;
    vec2_t cached_scale;
    vec2_t cached_skew;
} transform2d_t;

extern ecx_component_type Transform2D;
void Transform2D_setup(void);
#define get_transform2d(e) ((transform2d_t*)get_component(&Transform2D, e))
#define add_transform2d(e) ((transform2d_t*)add_component(&Transform2D, e))

#define get_world_transform2d(e) ((world_transform2d_t*)get_component_n(&Transform2D, e, 1))

void transform2d_set_matrix(transform2d_t* t, mat3x2_t m);
void transform2d_set_position_with_pivot(transform2d_t* t, vec2_t position, vec2_t pivot);
void transform2d_set_position_with_pivot_origin(transform2d_t* t, vec2_t position, vec2_t pivot, vec2_t origin);
void transform2d_update_matrix(transform2d_t* t);
void transform2d_scale(transform2d_t* t, float factor);
void transform2d_set_scale_f(transform2d_t* t, float value);
void transform2d_set_scale(transform2d_t* t, vec2_t value);
void transform2d_set_scale_x(transform2d_t* t, float x);
void transform2d_set_scale_y(transform2d_t* t, float y);
void transform2d_set_rotation(transform2d_t* t, float value);
float transform2d_get_rotation(const transform2d_t* t);
void transform2d_set_skew(transform2d_t* t, vec2_t value);
void transform2d_set_transform_ex(transform2d_t* t, vec2_t position, vec2_t scale, vec2_t skew, vec2_t pivot);
void transform2d_set_transform(transform2d_t* t, vec2_t position, vec2_t scale, float rotation);
void transform2d_set_transform_pss(transform2d_t* t, vec2_t position, vec2_t scale, vec2_t skew);
void transform2d_rotate(transform2d_t* t, float value);
void transform2d_translate(transform2d_t* t, vec2_t delta);
void transform2d_lerp_scale(transform2d_t* tr, vec2_t target, float t);
void transform2d_lerp_position(transform2d_t* tr, vec2_t target, float t);
void transform2d_lerp_rotation(transform2d_t* tr, float target, float t);

// API

// transform main properties
vec2_t get_position(entity_t e);

vec2_t get_scale(entity_t e);

float get_rotation(entity_t e);

void set_position(entity_t e, vec2_t v);

void set_scale(entity_t e, vec2_t v);

void set_rotation(entity_t e, float v);

void set_color(entity_t e, color_t color);

void set_color_offset(entity_t e, color_t offset);

// transform additional utilities
void set_alpha(entity_t e, uint8_t alpha);

void set_alpha_f(entity_t e, float alpha);

void set_color_additive_f(entity_t e, float additive);

void set_x(entity_t e, float x);

void set_y(entity_t e, float y);

void set_scale_f(entity_t e, float xy);

// spaces

vec2_t transform_up(entity_t it, entity_t top, vec2_t pos);

vec2_t transform_down(entity_t top, entity_t it, vec2_t pos);

vec2_t local_to_local(entity_t src, entity_t dst, vec2_t pos);

vec2_t local_to_global(entity_t local, vec2_t local_pos);

vec2_t global_to_local(entity_t local, vec2_t global_pos);

// these functions just helpers and use calculated world matrices, so use it only:
// - after transform invalidation phase
// - if node has own Transform2D components
void fast_local_to_local(entity_t src, entity_t dst, vec2_t pos, vec2_t* out);

/** system to invalidate matrix and color in world space **/
void update_world_transform_2d(entity_t root);


#ifdef __cplusplus
}
#endif

#endif // SCENEX_TRANSFORM2D_H
