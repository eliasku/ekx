#ifndef SCENEX_ATLAS_H
#define SCENEX_ATLAS_H

#include "sprite.h"

#include <ek/rr.h>
#include <ek/texture_loader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef ek_texture_loader* ek_texture_loader_ptr;

typedef struct {
    R(sprite_t)* sprites;
    R(sg_image)* pages;
    // loading routine
    ek_texture_loader_ptr* loaders;
    /* str_buf */ void* base_path;
    uint32_t format_mask;
    uint32_t state_flags;
} atlas_t;

void atlas_clear(atlas_t* atlas);

void atlas_load(atlas_t* atlas, const char* path, float scale_factor);

struct res_atlas {
    string_hash_t names[32];
    atlas_t data[32];
    rr_man_t rr;
};

extern struct res_atlas res_atlas;

void setup_res_atlas(void);

void update_res_atlas(void);

#define R_ATLAS(name) REF_NAME(res_atlas, name)

#ifdef __cplusplus
}
#endif

#endif // SCENEX_ATLAS_H
