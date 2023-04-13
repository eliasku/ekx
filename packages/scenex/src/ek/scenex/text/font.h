#ifndef SCENEX_FONT_H
#define SCENEX_FONT_H

#include <ek/math.h>
#include <ek/rr.h>
#include "font_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct font_base_ font_base_t;

typedef struct font_ {
    font_base_t* base;
    R(font_t) fallback;
} font_t;

void
font_draw(const font_t* font,
          const char* text,
          float size,
          vec2_t position,
          color_t color,
          float line_height,
          float line_spacing);

float font_get_text_segment_width(const font_t* font, const char* text, float size, int begin, int end);

font_type_t font_type(const font_t* font);

void font_set_fallback(font_t* font, string_hash_t fallbackFont);

bool font_get_glyph(font_t* font, uint32_t codepoint, glyph_t* outGlyph);

bool font_get_glyph_metrics(font_t* font, uint32_t codepoint, glyph_t* outGlyph);

void font_set_blur(font_t* font, float radius, int iterations, int strengthPower);

struct res_font {
    string_hash_t names[16];
    font_t data[16];
    rr_man_t rr;
};

extern struct res_font res_font;

void setup_res_font(void);

#define R_FONT(name) REF_NAME(res_font, name)

#ifdef __cplusplus
}
#endif

#endif // SCENEX_FONT_H
