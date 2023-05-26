#include <sce/dynamic_atlas.h>

#include <ek/assert.h>
#include <ek/buf.h>
#include <ek/gfx.h>
#include <string.h>

//void copy_pixels_normal(image_t& dest, int2 dest_position,
//                        const image_t& src, const rect_i& src_rect) {
//    rect_i dest_rc = {dest_position, src_rect.size};
//    rect_i src_rc = src_rect;
//    clip_rects(src.bounds<int>(), dest.bounds<int>(),
//               src_rc, dest_rc);
//
//    for (int32_t y = 0; y < src_rc.height; ++y) {
//        for (int32_t x = 0; x < src_rc.width; ++x) {
//            const auto pixel = get_pixel_unsafe(src, int2{src_rc.x + x, src_rc.y + y});
//            set_pixel_unsafe(dest, {dest_rc.x + x, dest_rc.y + y}, pixel);
//        }
//    }
//}

static dynamic_atlas_page_t create_page(int width, int height, bool alpha_map, bool mipmaps) {
    dynamic_atlas_page_t r = INIT_ZERO;
    r.width = width;
    r.height = height;
    r.invWidth = 1.0f / (float)width;
    r.invHeight = 1.0f / (float)height;
    r.bytesPerPixel = alpha_map ? 1 : 4;
    r.padding = 1;
    r.x = r.padding;
    r.y = r.padding;
    r.lineHeight = 0;
    r.dirtyRect = (irect_t){{0, 0, width, height}};
    r.dirty = true;
    r.alphaMap = alpha_map;
    r.mipmaps = mipmaps;

    r.dataSize = width * height * (alpha_map ? 1 : 4);
    r.data = (uint8_t*)calloc(1, r.dataSize);

    const sg_image_desc desc = {
        .type = SG_IMAGETYPE_2D,
        .width = width,
        .height = height,
        .usage = SG_USAGE_DYNAMIC,
        .pixel_format = alpha_map ? SG_PIXELFORMAT_R8 : SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
    r.image = sg_make_image(&desc);
    return r;
}

bool page_add_bitmap(dynamic_atlas_page_t* page, int spriteWidth, int spriteHeight, const uint8_t* pixelsData, size_t pixelsSize, dynamic_atlas_sprite_t* sprite) {
    EK_ASSERT(pixelsSize >= ((size_t)spriteWidth * spriteHeight * page->bytesPerPixel));
    EK_ASSERT(spriteWidth < page->width && spriteHeight < page->height);

    int placeX = page->x;
    int placeY = page->y;
    int newLineHeight = page->lineHeight;

    if (placeX + spriteWidth + page->padding > page->width) {
        placeX = 0;
        placeY += newLineHeight + page->padding;
        newLineHeight = 0;
    }
    if (placeY + spriteHeight > page->height) {
        // no
        return false;
    }

    sprite->image = page->image;
    sprite->texCoords = rect(page->invWidth * placeX,
                             page->invHeight * placeY,
                             page->invWidth * spriteWidth,
                             page->invHeight * spriteHeight);

    //texture->updateRect(placeX, placeY, spriteWidth, spriteHeight, pixels.data());

    {
        int srcStride = page->bytesPerPixel * spriteWidth;
        int destStride = page->bytesPerPixel * page->width;
        for (int cy = 0; cy < spriteHeight; ++cy) {
            memcpy(page->data + placeX + (placeY + cy) * destStride, pixelsData + cy * srcStride, srcStride);
        }
        if (!page->dirty) {
            page->dirtyRect = (irect_t){{placeX, placeY, spriteWidth, spriteHeight}};
        } else {
            page->dirtyRect = irect_combine(page->dirtyRect, (irect_t){{placeX, placeY, spriteWidth, spriteHeight}});
        }
        page->dirty = true;
    }

    if (newLineHeight < spriteHeight) {
        newLineHeight = spriteHeight;
    }
    placeX += spriteWidth + page->padding;

    page->x = placeX;
    page->y = placeY;
    page->lineHeight = newLineHeight;

    return true;
}

dynamic_atlas_sprite_t dynamic_atlas_add_bitmap(dynamic_atlas_t* atlas, int width, int height, const uint8_t* pixels, size_t pixels_size) {
    EK_ASSERT(pixels_size >= ((size_t)width * height * (atlas->alphaMap ? 1 : 4)));
    EK_ASSERT(width < atlas->pageWidth && height < atlas->pageHeight);

    dynamic_atlas_sprite_t sprite;
    arr_for(page, atlas->pages_) {
        if (page_add_bitmap(page, width, height, pixels, pixels_size, &sprite)) {
            return sprite;
        }
    }
    arr_push(atlas->pages_,
             create_page(atlas->pageWidth, atlas->pageHeight, atlas->alphaMap, atlas->mipmaps));
    dynamic_atlas_page_t* new_page = arr_back(atlas->pages_);

    if (!page_add_bitmap(new_page, width, height, pixels, pixels_size, &sprite)) {
        // how come?
        EK_ASSERT(false);
    }
    return sprite;
}

void dynamic_atlas_create(dynamic_atlas_t* atlas, int page_width, int page_height, bool alpha_map, bool mipmaps) {
    atlas->pageWidth = page_width;
    atlas->pageHeight = page_height;
    atlas->alphaMap = alpha_map;
    atlas->mipmaps = mipmaps;
    atlas->version = 0;
    arr_push(atlas->pages_, create_page(page_width, page_height, alpha_map, mipmaps));
}

void dynamic_atlas_destroy(dynamic_atlas_t* atlas) {
    arr_for(page, atlas->pages_) {
        free(page->data);
        sg_destroy_image(page->image);
    }
    arr_reset(atlas->pages_);
}

int dynamic_atlas_estimate_better_size(float scale_factor, uint32_t base_size, uint32_t max_size) {
    const uint32_t scaled_size = (uint32_t)ceilf((float)base_size * scale_factor);
    const uint32_t pot_size = round_up_pow2(scaled_size);
    return MIN(pot_size, max_size);
}

void dynamic_atlas_reset(dynamic_atlas_t* atlas) {
    arr_for(page, atlas->pages_) {
        memset(page->data, 0u, page->dataSize);
        page->dirtyRect = (irect_t){{0, 0, page->width, page->height}};
        page->dirty = true;
        page->x = page->padding;
        page->y = page->padding;
        page->lineHeight = 0;
    }
    ++atlas->version;
}

struct res_dynamic_atlas res_dynamic_atlas;

void setup_res_dynamic_atlas(void) {
    struct res_dynamic_atlas* R = &res_dynamic_atlas;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void update_res_dynamic_atlas(void) {
    #pragma nounroll
    for (res_id id = 1; id < res_dynamic_atlas.rr.num; ++id) {
        dynamic_atlas_t* atlas = &res_dynamic_atlas.data[id];
        arr_for(page, atlas->pages_) {
            if (page->dirty) {
                ek_gfx_update_image_0(page->image, page->data, page->dataSize);
                page->dirty = false;
            }
        }
    }
}
