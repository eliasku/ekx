#ifndef SCE_DYNAMIC_ATLAS_H
#define SCE_DYNAMIC_ATLAS_H

#include <ek/math.h>
#include <sokol/sokol_gfx.h>
#include <ek/hash.h>
#include <ek/rr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rect_t texCoords;
    sg_image image;
} dynamic_atlas_sprite_t;

typedef struct {
    sg_image image;
    int width;
    int height;
    float invWidth;
    float invHeight;
    int bytesPerPixel;
    int padding;
    int x;
    int y;
    int lineHeight;

    uint8_t* data;
    size_t dataSize;
    irect_t dirtyRect;
    bool dirty;
    bool alphaMap;
    bool mipmaps;
} dynamic_atlas_page_t;

typedef struct {
    dynamic_atlas_page_t* pages_;
    int pageWidth;
    int pageHeight;
    bool alphaMap;
    bool mipmaps;

    // after reset we increase version, so clients could check if cache maps should be cleared
    uint32_t version;
} dynamic_atlas_t;

void dynamic_atlas_create(dynamic_atlas_t* atlas, int page_width, int page_height, bool alpha_map, bool mipmaps);
void dynamic_atlas_destroy(dynamic_atlas_t* atlas);
dynamic_atlas_sprite_t
dynamic_atlas_add_bitmap(dynamic_atlas_t * atlas, int width, int height, const uint8_t* pixels, size_t pixels_size);

int dynamic_atlas_estimate_better_size(float scale_factor, uint32_t base_size, uint32_t max_size);

struct res_dynamic_atlas {
    string_hash_t names[16];
    dynamic_atlas_t data[16];
    rr_man_t rr;
};

extern struct res_dynamic_atlas res_dynamic_atlas;

void setup_res_dynamic_atlas(void);
void update_res_dynamic_atlas(void);

#define R_DYNAMIC_ATLAS(name) REF_NAME(res_dynamic_atlas, name)

#ifdef __cplusplus
}
#endif

#endif // SCE_DYNAMIC_ATLAS_H
