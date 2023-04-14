#ifndef SCENEX_FONT_TYPE_H
#define SCENEX_FONT_TYPE_H

#include <ek/gfx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    FONT_TYPE_BITMAP = 0,
    FONT_TYPE_TTF = 1,
} font_type_t;

typedef struct font_base_ font_base_t;

typedef struct {
    // multiply by fontSize
    rect_t rect;
    float advanceWidth;
    float bearingX;
    float lineHeight;
    float ascender;
    float descender;

    // sprite part
    rect_t texCoord;
    sg_image image;
    bool rotated;

    // we need to know what fallback give us this glyph info,
    // then we can ask additional info directly from source font implementation,
    // for example: kerning information
    font_base_t* source;
} glyph_t;


float font_base_kerning(font_base_t* font, uint32_t codepoint1, uint32_t codepoint2);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_FONT_TYPE_H
