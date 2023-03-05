#ifndef IMAGE_SET_H
#define IMAGE_SET_H

#include <ek/math.h>
#include <ek/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sprite_data_t {

    const char* name;

    // physical rect
    rect_t rc;

    // rect in source image
    irect_t source;

    // reference image;
    bitmap_t bitmap;

    uint8_t padding;

    bool trim;
} sprite_data_t;

typedef struct image_set_res_t {
    uint32_t index;
    float scale;
    sprite_data_t* sprites;
} image_set_res_t;

typedef struct image_set_t {
    image_set_res_t resolutions[8];
    uint32_t resolutions_num;
} image_set_t;

// TODO: move to ek/bitmap.h
void ek_bitmap_save_png(const bitmap_t* bitmap, const char* path, bool alpha);

// TODO: move to ek/bitmap.h
void ek_bitmap_save_jpg(const bitmap_t* bitmap, const char* path, bool alpha);

void save(image_set_t* bitmaps, const char* output);

// interop with C++ part
void export_xfl(const char* xfl_path, image_set_t* image_set, const char* output_path, const char* output_images_path);
void runFlashFilePrerender(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif
