#ifndef SCENEX_TRAIL2D_H
#define SCENEX_TRAIL2D_H

#include <ecx/ecx.h>
#include <ek/math.h>
#include <ek/rr.h>
#include <ekx/app/time_layers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    vec2_t position;
    float energy;
    float scale;
} trail2d_node_t;

typedef struct {
    // just storage for use
    uint32_t capacity;
    uint32_t end;
    uint32_t first;
    trail2d_node_t* data;
} trail2d_deque_t;

typedef struct {
    time_layer_t timer;
    vec2_t offset;
    float drain_speed;
    float segment_distance_max;
    float scale;
    vec2_t last_position;
    trail2d_deque_t nodes;
    bool initialized;
} trail2d_t;

void update_trail2d(void);

typedef struct {
    entity_t target;
    // max width
    float width;
    float minWidth;
    R(sprite_t) sprite;
} trail_renderer2d_t;

extern ecx_component_type Trail2D;
extern ecx_component_type TrailRenderer2D;

void setup_trail2d(void);

#define get_trail_renderer2d(e) ((trail_renderer2d_t*)get_component(&TrailRenderer2D, e))

trail_renderer2d_t*
add_trail_renderer2d(entity_t e, entity_t target);

#define get_trail2d(e) ((trail2d_t*)get_component(&Trail2D, e))
#define add_trail2d(e) ((trail2d_t*)get_component(&Trail2D, e))

#ifdef __cplusplus
}
#endif

#endif // SCENEX_TRAIL2D_H
