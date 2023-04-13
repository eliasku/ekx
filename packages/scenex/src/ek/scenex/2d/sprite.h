#ifndef SCENEX_SPRITE_H
#define SCENEX_SPRITE_H

#include <ek/gfx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    R_SPRITE_EMPTY = 1
};

enum sprite_flags_t {
    SPRITE_ROTATED = 1,
    SPRITE_LOADED = 2,
};

typedef struct sprite_t {
    uint32_t state;
    R(sg_image) image_id;
    rect_t rect;
    rect_t tex;
} sprite_t;

struct res_sprite {
    string_hash_t names[256];
    sprite_t data[256];
    rr_man_t rr;
};

extern struct res_sprite res_sprite;

void setup_res_sprite(void);

#define R_SPRITE(name) REF_NAME(res_sprite, name)

void draw_sprite(const sprite_t* sprite);

void draw_sprite_rc(const sprite_t* sprite, rect_t rc);

void draw_sprite_grid(const sprite_t* sprite, rect_t grid, rect_t target);

bool hit_test_sprite(const sprite_t* sprite, vec2_t position);

bool select_sprite(const sprite_t* sprite);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_SPRITE_H

