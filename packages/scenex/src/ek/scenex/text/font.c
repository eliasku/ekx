#include "font.h"
#include "ek/app.h"
#include <ek/buf.h>
#include <ek/bitmap.h>
#include <ek/canvas.h>
#include <ek/scenex/2d/dynamic_atlas.h>
#include <ek/scenex/2d/sprite.h>

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

#include <stb/stb_truetype.h>

#pragma clang diagnostic pop

#endif

// glyph map functions
static uint16_t* gm_find(glyph_map_t* map, uint64_t key) {
    EK_ASSERT(map != NULL);
    uint32_t i = map->head[key % 32];
    while (i) {
        EK_ASSERT(i - 1 < arr_size(map->entries));
        glyph_map_entry_t* entry = map->entries + i - 1;
        if (entry->key == key) {
            return &entry->val;
        }
        i = entry->next;
    }
    return NULL;
}

static void gm_set(glyph_map_t* map, uint64_t key, uint16_t val) {
    EK_ASSERT(map != NULL);
    const uint32_t m = key % 32;
    const glyph_map_entry_t entry = (glyph_map_entry_t) {key, val, map->head[m]};
    arr_push(map->entries, entry);
    map->head[m] = arr_size(map->entries);
}


// font resource pool
struct res_font res_font;

void setup_res_font(void) {
    struct res_font* R = &res_font;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

// font impl

static void ttf_reset_glyphs(font_t* font) {
    // clear dict
    arr_clear(font->map.entries);
    memset(font->map.head, 0, sizeof(font->map.head));

    // clear glyph list
    arr_clear(font->glyphs);

    // update dynamic atlas version
    if (font->atlas) {
        font->atlas_version = REF_RESOLVE(res_dynamic_atlas, font->atlas).version;
    }
}

static bool get_glyph_impl(font_t* font, uint32_t codepoint, glyph_t* outGlyph) {
    if (font->font_type == FONT_TYPE_BITMAP) {
        const uint16_t* p_index = gm_find(&font->map, codepoint);
        if (p_index) {
            bmfont_glyph_t* glyph = font->bmfont.glyphs + *p_index;
            outGlyph->advanceWidth = glyph->advance_x;
            outGlyph->lineHeight = font->line_height_multiplier;
            sprite_t* spr = &RES_NAME_RESOLVE(res_sprite, glyph->sprite);
            if (spr->state & SPRITE_LOADED) {
                outGlyph->rect = rect_scale_f(spr->rect, 1.0f / font->bmfont.header.base_font_size);
                outGlyph->texCoord = spr->tex;
                outGlyph->image = REF_RESOLVE(res_image, spr->image_id);
                outGlyph->rotated = spr->state & SPRITE_ROTATED;
            } else {
                outGlyph->rect = glyph->box;
            }
            outGlyph->source = font;
            return true;
        }
    } else if (font->font_type == FONT_TYPE_TTF) {
        // store pre-rendered glyph for baseFontSize and upscaled by dpi_scale
        // quad scale just multiplier to get fontSize relative to baseFontSize
        if (!font->loaded || !font->atlas) {
            return false;
        }

        dynamic_atlas_t* atlas_instance = &REF_RESOLVE(res_dynamic_atlas, font->atlas);
        if (arr_empty(atlas_instance->pages_)) {
            ttf_reset_glyphs(font);
            // not ready to fill dynamic atlas :(
            return false;
        } else if (font->atlas_version != atlas_instance->version) {
            ttf_reset_glyphs(font);
        }

        const uint64_t hash = font->effect_key_bits | codepoint;
        const uint16_t* it = gm_find(&font->map, hash);
        if (it) {
            *outGlyph = font->glyphs[*it];
            return true;
        }

        const int glyphIndex = stbtt_FindGlyphIndex(&font->info, codepoint);
        if (!glyphIndex) {
            /* @rlyeh: glyph not found, ie, arab chars */
            return false;
        }

        const float scale = stbtt_ScaleForPixelHeight(&font->info, font->base_font_size);
        glyph_t glyph = {0};
        gm_set(&font->map, hash, arr_size(font->glyphs));
        glyph.source = font;

        int advanceWidth = 0, leftSideBearing = 0;
        stbtt_GetGlyphHMetrics(&font->info, glyphIndex, &advanceWidth, &leftSideBearing);
        glyph.advanceWidth = scale * (float) (advanceWidth) / font->base_font_size;
        glyph.bearingX = scale * (float) (leftSideBearing) / font->base_font_size;

        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(&font->info, glyphIndex,
                                font->dpi_scale * scale, font->dpi_scale * scale, &x0, &y0, &x1, &y1);

        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            int pad = font->blur_radius;

            x0 -= pad;
            y0 -= pad;
            x1 += pad;
            y1 += pad;

            int bitmapWidth = x1 - x0;
            int bitmapHeight = y1 - y0;

            size_t bmpSize = bitmapWidth * bitmapHeight;
            uint8_t bmp[512 * 512];
            EK_ASSERT(bmpSize < 512 * 512);
            memset(bmp, 0, bmpSize);
            //uint8_t* bmp = (uint8_t*)calloc(1, bmpSize);

            stbtt_MakeGlyphBitmap(&font->info, bmp + pad * bitmapWidth + pad, glyphWidth, glyphHeight, bitmapWidth,
                                  font->dpi_scale * scale, font->dpi_scale * scale, glyphIndex);

            bitmap_blur_gray(bmp, bitmapWidth, bitmapHeight, bitmapWidth, font->blur_radius, font->blur_iterations,
                             font->strength_power);

            dynamic_atlas_sprite_t sprite = dynamic_atlas_add_bitmap(atlas_instance, bitmapWidth, bitmapHeight, bmp,
                                                                     bmpSize);
            //free(bmp);

            glyph.image = sprite.image;
            glyph.texCoord = sprite.texCoords;

            glyph.rect.x = x0;
            glyph.rect.y = y0;
            glyph.rect.w = bitmapWidth;
            glyph.rect.h = bitmapHeight;

            // scale to font size unit space
            glyph.rect = rect_scale_f(glyph.rect, 1.0f / (font->dpi_scale * font->base_font_size));
            glyph.lineHeight = font->line_height_multiplier;
            glyph.ascender = font->ascender;
            glyph.descender = font->descender;
        }
        arr_push(font->glyphs, glyph);
        *outGlyph = glyph;
        return true;
    }
    return false;
}

void font_draw(const font_t* font,
               const char* text,
               float size,
               vec2_t position,
               color_t color,
               float line_height,
               float line_spacing) {

    vec2_t current = position;
    vec2_t start = position;

    canvas_save_color();
    canvas_scale_color(color);

    uint32_t prev_image_id = SG_INVALID_ID;
    glyph_t gdata;
    for (; *text; ++text) {
        char code = *text;
        if (code == '\n') {
            current.x = start.x;
            current.y += line_height + line_spacing;
            continue;
        }

        if (get_glyph_impl((font_t*) font, code, &gdata)) {
            if (gdata.image.id) {
                if (prev_image_id != gdata.image.id) {
                    canvas_set_image(gdata.image);
                    prev_image_id = gdata.image.id;
                }
                canvas_set_image_rect(gdata.texCoord);
                gdata.rect = rect_scale_f(gdata.rect, size);
                if (gdata.rotated) {
                    canvas_quad_rotated(gdata.rect.x + current.x,
                                        gdata.rect.y + current.y,
                                        gdata.rect.w,
                                        gdata.rect.h);

                } else {
                    canvas_quad(gdata.rect.x + current.x,
                                gdata.rect.y + current.y,
                                gdata.rect.w,
                                gdata.rect.h);
                }
            }

            current.x += size * gdata.advanceWidth;
        }
    }
    canvas_restore_color();
}

float font_get_text_segment_width(const font_t* font, const char* text, float size, int begin, int end) {
    float x = 0.0f;
    float max = 0.0f;
    glyph_t gdata;
    for (int i = begin; i < end; ++i) {
        char c = text[i];
        if (c == '\n') {
            x = 0.0f;
        }
        if (get_glyph_impl((font_t*) font, text[i], &gdata)) {
            x += size * gdata.advanceWidth;
            if (max < x) {
                max = x;
            }
        }
    }
    return max;
}

bool font_get_glyph(font_t* font, uint32_t codepoint, glyph_t* outGlyph) {
    while (font) {
        if (get_glyph_impl(font, codepoint, outGlyph)) {
            return true;
        }
        font = font->fallback ? &REF_RESOLVE(res_font, font->fallback) : NULL;
    }
    return false;
}

void font_set_blur(font_t* font, float radius, int iterations, int strength_power) {
    while (font) {
        if (radius > 0.0f && iterations > 0) {
            radius = roundf(font->dpi_scale * radius);
            font->blur_radius = radius < 0 ? 0 : (radius > 0xFF ? 0xFF : (uint8_t) radius);
            font->blur_iterations = iterations > 3 ? 3 : iterations;
            font->strength_power = strength_power < 0 ? 0 : (strength_power > 7 ? 7 : strength_power);
            font->effect_key_bits =
                    (uint64_t) ((font->strength_power << 16) | (font->blur_iterations << 8) | font->blur_radius) << 32;
        } else {
            font->blur_radius = 0;
            font->blur_iterations = 0;
            font->strength_power = 0;
            font->effect_key_bits = 0;
        }

        font = font->fallback ? &REF_RESOLVE(res_font, font->fallback) : NULL;
    }
}

bool font_get_glyph_metrics(font_t* font, uint32_t codepoint, glyph_t* outGlyph) {
    while (font) {

        if (font->font_type == FONT_TYPE_BITMAP) {
            const uint16_t* index = gm_find(&font->map, codepoint);
            if (index) {
                bmfont_glyph_t* glyph = font->bmfont.glyphs + *index;
                outGlyph->advanceWidth = glyph->advance_x;
                outGlyph->lineHeight = font->line_height_multiplier;
                outGlyph->ascender = font->bmfont.header.ascender;
                outGlyph->descender = font->bmfont.header.descender;
                outGlyph->rect = glyph->box;
                outGlyph->source = font;
                return true;
            }
        } else if (font->font_type == FONT_TYPE_TTF) {
            if (!font->loaded) {
                return false;
            }

            const uint16_t* it = gm_find(&font->map, font->effect_key_bits | codepoint);
            if (it) {
                *outGlyph = font->glyphs[*it];
                return true;
            }

            const int glyphIndex = stbtt_FindGlyphIndex(&font->info, codepoint);
            if (!glyphIndex) {
                return false;
            }

            const float scale = stbtt_ScaleForPixelHeight(&font->info, font->base_font_size);
            int advanceWidth = 0, leftSideBearing = 0;
            stbtt_GetGlyphHMetrics(&font->info, glyphIndex, &advanceWidth, &leftSideBearing);
            outGlyph->advanceWidth = scale * (float) advanceWidth / font->base_font_size;
            outGlyph->bearingX = scale * (float) leftSideBearing / font->base_font_size;
            outGlyph->lineHeight = font->line_height_multiplier;
            outGlyph->ascender = font->ascender;
            outGlyph->descender = font->descender;

            int x0, y0, x1, y1;
            stbtt_GetGlyphBitmapBox(&font->info, glyphIndex, font->dpi_scale * scale, font->
                    dpi_scale * scale, &x0, &y0, &x1, &y1);
            outGlyph->rect.x = x0;
            outGlyph->rect.y = y0;
            outGlyph->rect.w = x1 - x0;
            outGlyph->rect.h = y1 - y0;
            outGlyph->rect = rect_scale_f(outGlyph->rect, 1.0f / (font->dpi_scale * font->base_font_size));

            outGlyph->source = font;
            return true;
        }

        font = font->fallback ? &REF_RESOLVE(res_font, font->fallback) : NULL;
    }
    return false;
}


float font_kerning(font_t* font, uint32_t codepoint1, uint32_t codepoint2) {
    if (font->loaded && font->font_type == FONT_TYPE_TTF && font->info.kern) {
        const int kern = stbtt_GetCodepointKernAdvance(&font->info, codepoint1, codepoint2);
        if (kern) {
            const float scale = stbtt_ScaleForPixelHeight(&font->info, font->base_font_size);
            return (float) kern * scale / font->base_font_size;
        }
    }
    return 0.0f;
}

////
void font_init(font_t* font, font_type_t fontType_) {
    *font = (font_t) {0};
    font->font_type = fontType_;
    font->line_height_multiplier = 1;
}

void ttf_unload(font_t* font);

void ttf_reset_glyphs(font_t* font);

// bmfont
void font_load_bmfont(font_t* font, const uint8_t* buffer, size_t length) {
    if (!length) {
        EK_ASSERT("can't load bmfont, buffer is empty");
        return;
    }
    calo_reader_t r = {0};
    r.p = (uint8_t*) buffer;
    read_calo(&r);
    font->bmfont = read_stream_bmfont(&r);
    font->line_height_multiplier = font->bmfont.header.line_height_multiplier;
    arr_for(entry, font->bmfont.dict) {
        gm_set(&font->map, entry->codepoint, entry->glyph_index);
    }

    font->ready = font->loaded = true;
}

// ttf font utils

void font_init_ttf(font_t* font, float dpiScale_, float fontSize, string_hash_t dynamicAtlasName) {
    font_init(font, FONT_TYPE_TTF);
    font->base_font_size = fontSize;
    font->dpi_scale = dpiScale_;
    font->atlas = R_DYNAMIC_ATLAS(dynamicAtlasName);
}

void font_destroy(font_t* font) {
    arr_reset(font->map.entries);
    if (font->font_type == FONT_TYPE_TTF) {
        ttf_unload(font);
    }
    font->loaded = false;
}

bool ttf_initFromMemory(font_t* font, const uint8_t* data, size_t size) {
    UNUSED(size);
    const uint8_t* font_data = data;
    const int font_idx = 0;
    const int font_offset = stbtt_GetFontOffsetForIndex(font_data, font_idx);
    EK_ASSERT(font_offset >= 0 && "fontData is incorrect, or fontIndex cannot be found.");
    if (stbtt_InitFont(&font->info, font_data, font_offset)) {
        int ascent, descent, lineGap;
        // Store normalized line height. The real line height is got
        // by multiplying the lineh by font size.
        stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
        int fh = ascent - descent;
        font->ascender = (float) ascent / (float) fh;
        font->descender = (float) descent / (float) fh;
        font->line_height_multiplier = (float) (fh + lineGap) / (float) fh;
        return true;
    }

    return false;
}

void ttf_loadFromMemory(font_t* font, ek_local_res* lr) {
    EK_ASSERT(!font->loaded);
    if (lr && lr->buffer && ttf_initFromMemory(font, lr->buffer, lr->length)) {
        font->source = *lr;
        font->loaded = true;
    }
}

void ttf_unload(font_t* font) {
    if (font->loaded) {
        arr_reset(font->glyphs);
        ek_local_res_close(&font->source);
        font->loaded = false;
    }
}

void on_ttf_loaded(ek_local_res* lr) {
    if (ek_local_res_success(lr)) {
        ttf_loadFromMemory((font_t*) lr->userdata, lr);
    } else {
        ek_local_res_close(lr);
    }
}

void font_load_device_font(font_t* font, const char* font_name) {
    EK_ASSERT(!font->loaded);
    char font_path[1024];
    if (0 == ek_app_font_path(font_path, sizeof(font_path), font_name) && font_path[0]) {
        ek_local_res_load(font_path, on_ttf_loaded, font);
    }
}
