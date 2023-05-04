#ifndef SCENEX_TEXT_FORMAT_H
#define SCENEX_TEXT_FORMAT_H

#include <ek/math.h>
#include <ek/hash.h>
#include <ek/rr.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ALIGNMENT_NONE = 0,
    ALIGNMENT_LEFT = 1,
    ALIGNMENT_TOP = 2,
    ALIGNMENT_RIGHT = 4,
    ALIGNMENT_BOTTOM = 8,
    ALIGNMENT_CENTER_X = 16,
    ALIGNMENT_CENTER_Y = 32,
    ALIGNMENT_CENTER = ALIGNMENT_CENTER_X | ALIGNMENT_CENTER_Y,
    ALIGNMENT_TOP_LEFT = ALIGNMENT_TOP | ALIGNMENT_LEFT,
    ALIGNMENT_CENTER_BOTTOM = ALIGNMENT_CENTER_X | ALIGNMENT_BOTTOM
};

typedef int32_t alignment_t;

typedef enum {
    TEXT_LAYER_TEXT = 0,
    TEXT_LAYER_STROKE1 = 1,
    TEXT_LAYER_STROKE2 = 2,
    TEXT_LAYER_SHADOW = 3,
    TEXT_LAYERS_MAX = 4,
} text_layer_type;

// type + color + offset = 16
// blur + effects = 8
// Total: 24 bytes
typedef struct text_layer_effect_ {
    text_layer_type type;
    color_t color;
    vec2_t offset;
    float blurRadius;
    uint8_t blurIterations;
    uint8_t strength;
    bool visible;
    bool showGlyphBounds;
} text_layer_effect_t;

// 8 + 4 + 4 + 4 + 8 + 4(flags) = 32
// layers = 24 bytes * 4 (LayersMax) = 96
// count / indx = 8
// Total = 136 bytes
typedef struct text_format_ {
    res_id/*(font_t)*/ font;
    float size;
    float leading;
    float letterSpacing;

    // not actually box alignment , but just to be shared for single line
    vec2_t alignment;

    bool kerning;
    bool underline;
    bool strikethrough;
    bool wordWrap;
    bool allowLetterWrap;

    text_layer_effect_t layers[TEXT_LAYERS_MAX];
    int layersCount;
    int textLayerIndex;
} text_format_t;

void
text_format_set_text_color(text_format_t* tf, color_t color);

color_t
text_format_get_text_color(const text_format_t* tf);

void
text_format_set_font(text_format_t* tf, string_hash_t font_name);

void
text_format_align(text_format_t* tf, alignment_t align);

// hardness = 0.2f /* 0..1 */
// offset = vec2(0, 0)
void
text_format_add_shadow(text_format_t* tf, color_t color, float radius, vec2_t offset, float hardness);

text_format_t
text_format(string_hash_t font_name, float size);

vec2_t
vec2_ui_align(alignment_t alignment, vec2_t v);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_TEXT_FORMAT_H
