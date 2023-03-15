#include "image_set.h"
#include <ek/buf.h>
#include <ek/print.h>
#include <ek/assert.h>
#include <ek/hash.h>
#include <miniz.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBIW_ASSERT(e)   EK_ASSERT(e)

unsigned char* iwcompress(unsigned char* data, int data_len, int* out_len, int quality) {
    // uber compression
    quality = 10;
    mz_ulong buflen = mz_compressBound(data_len);
    uint8_t* buf = (unsigned char*) malloc(buflen);
    if (mz_compress2(buf, &buflen, data, data_len, quality)) {
        free(buf);
        return 0;
    }
    *out_len = (int) buflen;
    return buf;
}

#define STBIW_ZLIB_COMPRESS(a, b, c, d)  iwcompress(a,b,c,d)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#include <stb/stb_image_write.h>
#include <stb/stb_sprintf.h>

#pragma clang diagnostic pop

#endif

/*** Save Image ***/

void ek_bitmap_save_png(const bitmap_t* bitmap, const char* path, bool alpha) {
    EK_ASSERT(bitmap != NULL);
    EK_ASSERT(bitmap->pixels != NULL);

    const int w = (int) bitmap->w;
    const int h = (int) bitmap->h;

    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);

    stbi_write_png_compression_level = 10;
    stbi_write_force_png_filter = 0;

    if (alpha) {
        stbi_write_png(path, w, h, 4, bitmap->pixels, 4 * w);
    } else {
        const size_t pixels_count = w * h;
        uint8_t* buffer = (uint8_t*) malloc(pixels_count * 3);
        uint8_t* buffer_rgb = buffer;
        const uint8_t* buffer_rgba = (const uint8_t*) bitmap->pixels;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_png(path, w, h, 3, buffer, 3 * w);
        free(buffer);
    }
}

void ek_bitmap_save_jpg(const bitmap_t* bitmap, const char* path, bool alpha) {
    // require RGBA non-premultiplied alpha
    //undo_premultiply_image(img);
    const int w = (int) bitmap->w;
    const int h = (int) bitmap->h;
    const size_t pixels_count = w * h;
    const uint8_t* buffer_rgba = (uint8_t*) bitmap->pixels;

    if (alpha) {
        uint8_t* buffer_rgb = (uint8_t*) malloc(pixels_count * 3);
        uint8_t* buffer_alpha = (uint8_t*) malloc(pixels_count);

        uint8_t* rgb = buffer_rgb;
        uint8_t* alphaMask = buffer_alpha;
        for (size_t i = 0; i < pixels_count; ++i) {
            rgb[0] = buffer_rgba[0];
            rgb[1] = buffer_rgba[1];
            rgb[2] = buffer_rgba[2];
            alphaMask[0] = buffer_rgba[3];
            buffer_rgba += 4;
            rgb += 3;
            alphaMask += 1;
        }

        char outputPath[1024];
        ek_snprintf(outputPath, sizeof(outputPath), "%s.jpg", path);
        stbi_write_jpg(outputPath, w, h, 3, buffer_rgb, 90);

        ek_snprintf(outputPath, sizeof(outputPath), "%s_a.jpg", path);
        stbi_write_jpg(outputPath, w, h, 1, buffer_alpha, 90);

        free(buffer_rgb);
        free(buffer_alpha);
    } else {
        uint8_t* buffer = (uint8_t*) malloc(pixels_count * 3);
        uint8_t* buffer_rgb = buffer;

        for (size_t i = 0; i < pixels_count; ++i) {
            buffer_rgb[0] = buffer_rgba[0];
            buffer_rgb[1] = buffer_rgba[1];
            buffer_rgb[2] = buffer_rgba[2];
            buffer_rgba += 4;
            buffer_rgb += 3;
        }

        stbi_write_jpg(path, w, h, 3, buffer, 90);

        free(buffer);
    }
}

void save(image_set_t* images, const char* output) {
    char path[1024];
    snprintf(path, sizeof path, "%s/images.txt", output);
    FILE* f = fopen(path, "wb");
    int idx = 0;

    for (uint32_t i = 0; i < images->resolutions_num; ++i) {
        image_set_res_t* res = images->resolutions + i;
        uint32_t numImages = 0;
        for (uint32_t image_index = 0, images_num = arr_size(res->sprites); image_index < images_num; ++image_index) {
            sprite_data_t* image = res->sprites + image_index;
            if (image->bitmap.pixels) {
                ++numImages;
            }
        }
        fprintf(f, "%u\n", numImages);
        for (uint32_t image_index = 0, images_num = arr_size(res->sprites); image_index < images_num; ++image_index) {
            sprite_data_t* image = res->sprites + image_index;
            if (image->bitmap.pixels) {
                snprintf(path, sizeof path, "%s/%d.bmp", output, idx++);
                const char* imagePath = path;
                fprintf(f, "%u\n%s\n%s\n%f %f %f %f %u %u\n",
                        H(image->name),
                        image->name,
                        imagePath,
                        image->rc.x, image->rc.y, image->rc.w, image->rc.h,
                        image->padding, 0);
                {
                    bitmap_t bitmap = image->bitmap;
                    // require RGBA non-premultiplied alpha
                    bitmap_unpremultiply(&bitmap);
                    stbi_write_bmp(imagePath, bitmap.w, bitmap.h, 4, bitmap.pixels);
                }
            }
        }
    }
    fclose(f);
}
