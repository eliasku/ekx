#include "text_engine.h"
#include "font.h"
#include "font_base.h"
#include <ek/buf.h>
#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/utf8.h>
#include <ek/print.h>
#include <stdarg.h>

text_engine_t text_engine;

void setup_text_engine(void) {
    text_engine = {};
    text_engine.format = text_format(H("mini"), 16.0f);
}

static font_t* get_font_data(R(Font) ref) {
    return ref ? &REF_RESOLVE(res_font, ref) : nullptr;
}

static void update_line_size(vec2_t* line_size, float length, float height) {
    if (line_size->x < length) {
        line_size->x = length;
    }
    if (line_size->y < height) {
        line_size->y = height;
    }
}

static void close_line(text_block_line_t* line, float empty_line_height, int end) {
    line->end = end;
    if (line->size.y <= 0.0f) {
        line->size.y = empty_line_height;
    }
}

static const int WarningLinesCount = 128;

static void add_line(text_block_t* block, text_block_line_t line) {
    EK_ASSERT(arr_size(block->lines) < WarningLinesCount);
    if (block->size.x < line.size.x) {
        block->size.x = line.size.x;
    }
    block->size.y += line.size.y;
    if (arr_size(block->lines)) {
        block->size.y += arr_back(block->lines)->leading;
    }
    arr_push(block->lines, line);
}

static void reset_text_block(text_block_t* block) {
    block->size = vec2(0, 0);
    arr_clear(block->lines);
}

//static void scale_text_block(text_block_t* block, float factor) {
//    block->size *= factor;
//    arr_for(line, block->lines) {
//        line->size *= factor;
//        // for first-line position
//        line->ascender *= factor;
//        //line.descender *= factor;
//    }
//}

static bool check_text_block_valid(const text_block_t* block) {
    int pos = 0;
    arr_for (line, block->lines) {
        if (line->end < line->begin) {
            EK_ASSERT(false);
            return false;
        }
        if (line->begin < pos) {
            EK_ASSERT(false);
            return false;
        }
        if (line->end < pos) {
            EK_ASSERT(false);
            return false;
        }
        pos = line->end;
    }
    return true;
}

void draw_text(const char* text) {
    if (!text_engine.format.font) {
        return;
    }
    text_block_t* info = &text_engine.text_block;
    get_text_block_size(text, info);
    draw_text_block(text, info);
}

void draw_text_block(const char* text, const text_block_t* info) {
    const font_t* font = get_font_data(text_engine.format.font);
    if (!font) {
        return;
    }
    //auto alignment = format.alignment;

    canvas_push_program(res_shader.data[R_SHADER_ALPHA_MAP]);
    // render effects first
    for (int i = text_engine.format.layersCount - 1; i >= 0; --i) {
        auto* layer = text_engine.format.layers + i;
        if (!layer->visible) {
            continue;
        }
        if (font_type(font) == FONT_TYPE_BITMAP && layer->blurRadius > 0.0f &&
            length_sqr_vec2(layer->offset) <= 0.1f) {
            // skip {0;0} strokes for bitmap fonts
            continue;
        }
        draw_text_block_layer(text, layer, info);
    }
    canvas_restore_program();
}

void draw_text_block_layer(const char* text, const text_layer_effect_t* layer, const text_block_t* info) {
    text_format_t format = text_engine.format;
    font_t* font = get_font_data(format.font);
    if (!font) {
        return;
    }

    vec2_t alignment = format.alignment;
    float size = format.size;
    float letterSpacing = format.letterSpacing;
    bool kerning = format.kerning;

    font_set_blur(font, layer->blurRadius, layer->blurIterations, layer->strength);

    vec2_t current = text_engine.position + layer->offset;
    const float startX = current.x;
    int lineIndex = 0;
    int numLines = (int) arr_size(info->lines);

    current.x += (info->size.x - info->lines[lineIndex].size.x) * alignment.x;

    canvas_save_color();
    canvas_scale_color(layer->color);

    uint32_t prev_image_id = SG_INVALID_ID;
    uint32_t prevCodepointOnLine = 0;
    glyph_t gdata;
    uint32_t codepoint = 0;
    while (lineIndex < numLines) {
        const char* it = text + info->lines[lineIndex].begin;
        const char* end = text + info->lines[lineIndex].end;
        while (it != end) {
            codepoint = utf8_next(&it);
            if (font_get_glyph(font, codepoint, &gdata)) {
                if (kerning && prevCodepointOnLine) {
                    current.x += gdata.source->getKerning(prevCodepointOnLine, codepoint) * size;
                }
                if (gdata.image.id) {
                    if (prev_image_id != gdata.image.id) {
                        canvas_set_image(gdata.image);
                        prev_image_id = gdata.image.id;
                    }
                    canvas_set_image_rect(gdata.texCoord);
                    gdata.rect = rect_translate(rect_scale_f(gdata.rect, size), current);
                    if (!gdata.rotated) {
                        canvas_quad(gdata.rect.x,
                                    gdata.rect.y,
                                    gdata.rect.w,
                                    gdata.rect.h);
                    } else {
                        canvas_quad_rotated(gdata.rect.x,
                                            gdata.rect.y,
                                            gdata.rect.w,
                                            gdata.rect.h);
                    }
                    // only for DEV mode
                    if (layer->showGlyphBounds) {
                        canvas_set_empty_image();
                        canvas_stroke_rect(gdata.rect, COLOR_WHITE, 1);
                        canvas_set_image(gdata.image);
                    }
                }

                current.x += size * gdata.advanceWidth + letterSpacing;
            }
            prevCodepointOnLine = codepoint;
        }
        current.x = startX;
        current.y += info->lines[lineIndex].size.y;
        ++lineIndex;
        if (lineIndex < numLines) {
            current.x += (info->size.x - info->lines[lineIndex].size.x) * alignment.x;
            current.y += info->lines[lineIndex - 1].leading;
            prevCodepointOnLine = 0;
        }
    }

    canvas_restore_color();
}

// TextEngineUtils

static const char* whitespaces = " \n\t";
static const char* punctuations = ".,!?:";

static bool in_ascii_range(uint32_t c, const char* range) {
    return c && c == (c & 0x7Fu) && strchr(range, static_cast<char>(c & 0x7Fu));
}

static const char* skip(const char* it, const char* range) {
    const char* prev = it;
    uint32_t c = utf8_next(&it);
    while (in_ascii_range(c, range)) {
        prev = it;
        c = utf8_next(&it);
    }
    return prev;
}

static bool is_whitespace(uint32_t c) {
    return in_ascii_range(c, whitespaces);
}

static bool is_punctuation(uint32_t c) {
    return in_ascii_range(c, punctuations);
}

static const char* skip_whitespaces(const char* it) {
    return skip(it, whitespaces);
}

void get_text_block_size(const char* text, text_block_t* info) {
    text_format_t format = text_engine.format;
    reset_text_block(info);

    font_t* font = get_font_data(format.font);
    if (!font) {
        return;
    }

    float size = format.size;
    float leading = format.leading;
    bool kerning = format.kerning;
    float letterSpacing = format.letterSpacing;
    //auto alignment = format.alignment;

    float x = 0.0f;
    glyph_t metrics;
    const char* it = text;
    const char* prev = text;
    const char* lastWrapToPosition = nullptr;
    text_block_line_t lastWrapLine;

    uint32_t prevCodepointOnLine = 0;
    uint32_t codepoint = 0;
    text_block_line_t line = INIT_ZERO;
    codepoint = utf8_next(&it);
    while (codepoint) {
        if (codepoint == '\n') {
            close_line(&line, size, static_cast<int>(prev - text));
            add_line(info, line);
            line = INIT_ZERO;
            line.begin = static_cast<int>(it - text);
            x = 0.0f;

            lastWrapLine = INIT_ZERO;
            // next char
            prevCodepointOnLine = 0;
            prev = it;
            codepoint = utf8_next(&it);
            continue;
        }
        // wordwrap
        if (format.wordWrap) {
            if (is_punctuation(prevCodepointOnLine)) {
                lastWrapToPosition = skip_whitespaces(prev);
                lastWrapLine = line;
                close_line(&lastWrapLine, size, static_cast<int>(prev - text));
            } else if (is_whitespace(codepoint)) {
                lastWrapToPosition = skip_whitespaces(it);
                lastWrapLine = line;
                close_line(&lastWrapLine, size, static_cast<int>(prev - text));
            }
        }
        if (font_get_glyph_metrics(font, codepoint, &metrics)) {
            const float kern = (kerning && prevCodepointOnLine) ?
                               metrics.source->getKerning(prevCodepointOnLine, codepoint) * size :
                               0.0f;
            auto right = x + kern + size * fmax(RECT_R(metrics.rect), metrics.advanceWidth);
            if (format.wordWrap && right > text_engine.max_width && text_engine.max_width > 0) {
                if (lastWrapLine.end != 0) {
                    add_line(info, lastWrapLine);
                    line = INIT_ZERO;
                    it = lastWrapToPosition;
                    line.begin = static_cast<int>(it - text);
                    x = 0.0f;
                    lastWrapLine = INIT_ZERO;
                    prevCodepointOnLine = 0;
                    prev = it;
                    codepoint = utf8_next(&it);
                    continue;
                }
                    // at least one symbol added to line
                else if (it > text + line.begin && format.allowLetterWrap) {
                    close_line(&line, size, static_cast<int>(prev - text));
                    add_line(info, line);
                    line = INIT_ZERO;
                    lastWrapLine = INIT_ZERO;
                    line.begin = static_cast<int>(prev - text);
                    x = 0.0f;
                    prevCodepointOnLine = 0;
                    // use current codepoint
                    continue;
                }
            }
            if (line.ascender < size * metrics.ascender) {
                line.ascender = size * metrics.ascender;
            }
            if (line.descender < size * metrics.descender) {
                line.descender = size * metrics.descender;
            }
            line.leading = leading;
            update_line_size(&line.size, right, size * metrics.lineHeight);
            x += kern + size * metrics.advanceWidth + letterSpacing;
        }
        prevCodepointOnLine = codepoint;
        prev = it;
        codepoint = utf8_next(&it);
    }
    close_line(&line, size, static_cast<int>(prev - text));
    add_line(info, line);

    EK_ASSERT(check_text_block_valid(info));
}

void draw_text_format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    ek_vsnprintf(buffer, sizeof buffer, fmt, args);
    va_end(args);

    draw_text(buffer);
}

