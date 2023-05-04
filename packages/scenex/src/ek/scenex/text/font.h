#ifndef SCENEX_FONT_H
#define SCENEX_FONT_H

#include <ek/gfx.h>
#include <ek/local_res.h>
#include <ek/math.h>
#include <ek/rr.h>
#include <gen_sg.h>
#include <stb/stb_truetype.h>

#ifdef __cplusplus
extern "C" {
#endif

// Glyph Map structure for hashing codepoint-glyph

typedef struct {
    uint64_t key;
    uint16_t val;
    uint16_t next;
} glyph_map_entry_t;

typedef struct {
    uint16_t head[32];
    glyph_map_entry_t* entries;
} glyph_map_t;

// glyph structure

typedef enum {
    FONT_TYPE_BITMAP = 0,
    FONT_TYPE_TTF = 1,
    FONT_TYPES_COUNT = 2,
} font_type_t;

typedef struct font_ font_t;

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
    font_t* source;
} glyph_t;

// Font resource
struct font_ {
    font_type_t font_type;

    /* R(font_t) */
    res_id fallback;

    float line_height_multiplier;

    bool loaded;
    bool ready;

    // dynamic hash-map to map 64-bit key(effect+codepoint) to 16-bit glyph index
    glyph_map_t map;

    uint64_t effect_key_bits;
    uint8_t blur_radius;
    uint8_t blur_iterations;
    uint8_t strength_power;

    union {
        // bmfont is mapped to memory, we use this control structure with mapped pointers as source of data
        bmfont_t bmfont;

        struct {
            stbtt_fontinfo info;
            ek_local_res source;

            float base_font_size;
            float dpi_scale;
            res_id atlas;
            uint32_t atlas_version;
            // pre-rendered dynamic glyphs or glyph effects
            glyph_t* glyphs;

            float ascender;
            float descender;
        };
    };
};

void font_draw(const font_t* font,
               const char* text,
               float size,
               vec2_t position,
               color_t color,
               float line_height,
               float line_spacing);

float font_get_text_segment_width(const font_t* font, const char* text, float size, int begin, int end);

bool font_get_glyph(font_t* font, uint32_t codepoint, glyph_t* out_glyph);

bool font_get_glyph_metrics(font_t* font, uint32_t codepoint, glyph_t* out_glyph);

void font_set_blur(font_t* font, float radius, int iterations, int strength_power);

float font_kerning(font_t* font, uint32_t codepoint1, uint32_t codepoint2);

// base font implementation

void font_load_bmfont(font_t* font, const uint8_t* buffer, size_t length);

void font_destroy(font_t* font);

void font_init(font_t* font, font_type_t fontType_);

void font_init_ttf(font_t* font, float dpi_scale, float fontSize, string_hash_t dynamic_atlas_name);

void ttf_loadFromMemory(font_t* font, ek_local_res* lr);

void font_load_device_font(font_t* font, const char* font_name);

// font resource pool

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
