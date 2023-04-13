#include "font.h"

#include "font_base.h"
#include <ek/canvas.h>

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

        if (font->base->getGlyph(code, &gdata)) {
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
        auto c = text[i];
        if (c == '\n') {
            x = 0.0f;
        }
        if (font->base->getGlyph(text[i], &gdata)) {
            x += size * gdata.advanceWidth;
            if (max < x) {
                max = x;
            }
        }
    }
    return max;
}

font_type_t font_type(const font_t* font) {
    return font->base->getFontType();
}

bool font_get_glyph(font_t* font, uint32_t codepoint, glyph_t* outGlyph) {
    if (font->base->getGlyph(codepoint, outGlyph)) {
        return true;
    }
    if (font->fallback) {
        font_t* fb = &REF_RESOLVE(res_font, font->fallback);
        if (fb && font_get_glyph(fb, codepoint, outGlyph)) {
            return true;
        }
    }
    return false;
}

void font_set_blur(font_t* font, float radius, int iterations, int strengthPower) {
   font->base->setBlur(radius, iterations, strengthPower);
    if (font->fallback) {
        font_t* fb = &REF_RESOLVE(res_font, font->fallback);
        if(fb) {
            font_set_blur(fb, radius, iterations, strengthPower);
        }
    }
}

bool font_get_glyph_metrics(font_t* font, uint32_t codepoint, glyph_t* outGlyph) {
    if (font->base->getGlyphMetrics(codepoint, outGlyph)) {
        return true;
    }
    if (font->fallback) {
        font_t* fb = &REF_RESOLVE(res_font, font->fallback);
        if (fb && font_get_glyph_metrics(fb, codepoint, outGlyph)) {
            return true;
        }
    }
    return false;
}

void font_set_fallback(font_t* font, string_hash_t fallback_font_name) {
    font->fallback = R_FONT(fallback_font_name);
}

