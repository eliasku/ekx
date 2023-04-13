#ifndef SCENEX_TEXT_ENGINE_H
#define SCENEX_TEXT_ENGINE_H

#include "font.h"
#include "text_format.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int begin;
    int end;
    vec2_t size;
    float ascender;
    float descender;
    float leading;
} text_block_line_t;

typedef struct {
    // {max length; total height}
    vec2_t size;
    text_block_line_t* lines;
} text_block_t;

typedef struct {
    // shared text block info data
    text_block_t text_block;

    // user set
    text_format_t format;
    float max_width;

    // current pen position
    vec2_t position;

    // drawing zone
    rect_t bounds;

    // alignment for rect
    // TODO:
    vec2_t bounds_alignment;
} text_engine_t;

void draw_text(const char* text);

void draw_text_format(const char* fmt, ...);

void draw_text_block(const char* text, const text_block_t* info);

void draw_text_block_layer(const char* text, const text_layer_effect_t* layer, const text_block_t* info);

// bounds only for text lines (no glyph metrics or effect pixels)
void get_text_block_size(const char* text, text_block_t* info);

extern text_engine_t text_engine;

void setup_text_engine(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_TEXT_ENGINE_H
