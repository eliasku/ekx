#ifndef SCENEX_DISPLAY2D_H
#define SCENEX_DISPLAY2D_H

#include <ek/math.h>
#include <ek/gfx.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BOUNDS_2D_HIT_AREA = 1,
    BOUNDS_2D_SCISSORS = 2,
    BOUNDS_2D_CULL = 4,
} bounds2d_flags_t;

typedef struct {
    rect_t rect;
    uint32_t flags;
} bounds2d_t;

rect_t get_world_rect(const bounds2d_t* bounds, mat3x2_t world_matrix);

rect_t get_screen_rect(const bounds2d_t* bounds, mat3x2_t view_matrix, mat3x2_t world_matrix);

typedef struct {
    // state management
    R(ek_shader) program;

    // 1 - draw debug bounds
    uint32_t flags;

    void (* draw)(entity_t e);

    // TODO: rename as post draw
    void (* callback)(entity_t e);

    bool (* hit_test)(entity_t e, vec2_t local_pos);

    rect_t (* get_bounds)(entity_t e);
} display2d_t;

// 16 + 16 = 32 bytes
typedef struct {
    R(sprite_t) src;
    rect_t rect;
    color_t colors[4];
} quad2d_t;

// 8 + 1 = 9 bytes
typedef struct {
    R(sprite_t) src;
    bool hit_pixels;
} sprite2d_t;

// 8 + 16 + 16 + 8 + 1 = 49 bytes
typedef struct  {
    R(sprite_t) src;
    rect_t scale_grid;
    rect_t manual_target;
    vec2_t scale;
    bool hit_pixels;

//    NinePatch2D(string_hash_t spriteId, rect_t aScaleGrid) :
//            src{R_SPRITE(spriteId)},
//            scale_grid{aScaleGrid} {
//    }

} ninepatch2d_t;

// 4 + 4 + 4 + 4 + 4 + 4 + 8 = 32 bytes
typedef struct {
    float angle;
    float radius;
    float line_width;
    int segments;
    color_t color_inner;
    color_t color_outer;
    R(sprite_t) sprite;
} arc2d_t;

extern ecx_component_type Bounds2D;
extern ecx_component_type Display2D;
extern ecx_component_type Quad2D;
extern ecx_component_type Sprite2D;
extern ecx_component_type NinePatch2D;
extern ecx_component_type Arc2D;

void Display2D_setup(void);

#define get_bounds2d(e) ((bounds2d_t*)get_component(&Bounds2D, e))
#define add_bounds2d(e) ((bounds2d_t*)add_component(&Bounds2D, e))

#define get_display2d(e) ((display2d_t*)get_component(&Display2D, e))
#define add_display2d(e) ((display2d_t*)add_component(&Display2D, e))

#define get_quad2d(e) ((quad2d_t*)get_component(&Quad2D, e))
#define add_quad2d(e) ((quad2d_t*)add_component(&Quad2D, e))

#define get_sprite2d(e) ((sprite2d_t*)get_component(&Sprite2D, e))
#define add_sprite2d(e) ((sprite2d_t*)add_component(&Sprite2D, e))

#define get_ninepatch2d(e) ((ninepatch2d_t*)get_component(&NinePatch2D, e))
#define add_ninepatch2d(e) ((ninepatch2d_t*)add_component(&NinePatch2D, e))

#define get_arc2d(e) ((arc2d_t*)get_component(&Arc2D, e))
#define add_arc2d(e) ((arc2d_t*)add_component(&Arc2D, e))

quad2d_t* quad2d_setup(entity_t e);

sprite2d_t* sprite2d_setup(entity_t e);

ninepatch2d_t* ninepatch2d_setup(entity_t e);

arc2d_t* arc2d_setup(entity_t e);

/// drawables callbacks

void quad2d_draw(entity_t e);

rect_t quad2d_get_bounds(entity_t e);

bool quad2d_hit_test(entity_t e, vec2_t lp);

void set_gradient_quad(entity_t e, rect_t rc, color_t top, color_t bottom);

void set_color_quad(entity_t e, rect_t rc, color_t color);

void sprite2d_draw(entity_t e);

rect_t sprite2d_get_bounds(entity_t e);

bool sprite2d_hit_test(entity_t e, vec2_t lp);

void ninepatch2d_draw(entity_t e);

rect_t ninepatch2d_get_bounds(entity_t e);

bool ninepatch2d_hit_test(entity_t e, vec2_t lp);

void arc2d_draw(entity_t e);

rect_t arc2d_get_bounds(entity_t e);

bool arc2d_hit_test(entity_t e, vec2_t lp);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_DISPLAY2D_H
