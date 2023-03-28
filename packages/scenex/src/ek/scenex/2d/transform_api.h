#ifndef SCENEX_TRANSFORM_2D_API_H
#define SCENEX_TRANSFORM_2D_API_H

#include <ek/math.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#endif // SCENEX_TRANSFORM_2D_API_H