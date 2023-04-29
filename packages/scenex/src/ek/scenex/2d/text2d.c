#include "text2d.h"

#include "display2d.h"
#include <ek/canvas.h>
#include <ek/scenex/text/text_engine.h>
#include <ekx/app/localization.h>
#include <ek/print.h>
#include <ek/buf.h>
#include <ek/string.h>

ecx_component_type Text2D;

static void text2d_ctor(component_handle_t i) {
    text2d_t txt = {0};
    txt.format = text_format(H("mini"), 16.0f);
    ((text2d_t*) Text2D.data[0])[i] = txt;
}

static void text2d_dtor(component_handle_t i) {
    void* str_obj = ((text2d_t*) Text2D.data[0])[i].str_obj;
    arr_reset(str_obj);
}

void setup_text2d(void) {
    ECX_TYPE(text2d_t, 16);
    Text2D.ctor = text2d_ctor;
    Text2D.dtor = text2d_dtor;
}

static float find_text_scale(vec2_t textSize, rect_t rc) {
    float textScale = 1.0f;
    if (rc.w > 0.0f && textSize.x > 0.0f) {
        textScale = MIN(textScale, rc.w / textSize.x);
    }
    if (rc.h > 0.0f && textSize.y > 0.0f) {
        textScale = MIN(textScale, rc.h / textSize.y);
    }
    return textScale;
}

static void adjust_font_size(const char* text, rect_t bounds) {
    text_block_t* text_block = &text_engine.text_block;
    const float minFontSize = 10.0f;

    text_engine.format.allowLetterWrap = false;

    get_text_block_size(text, text_block);

    if (text_engine.format.wordWrap) {
        while (text_engine.format.size > minFontSize &&
               (text_block->size.x > bounds.w || text_block->size.y > bounds.h)) {
            text_engine.format.size -= 1.0f;
            get_text_block_size(text, text_block);
        }
    } else {
        const float textScale = find_text_scale(text_block->size, bounds);
        if (textScale < 1.0f) {
            text_engine.format.size *= textScale;
            if (text_engine.format.size < minFontSize) {
                text_engine.format.size = minFontSize;
            }
        }
    }
    text_engine.format.allowLetterWrap = true;
}

void text2d_draw(entity_t e) {
    text2d_t* d = get_text2d(e);
    text_block_t* text_block = &text_engine.text_block;
    text_engine.format = d->format;
    text_engine.max_width = d->format.wordWrap ? d->rect.w : 0.0f;
    if (d->fillColor.a > 0) {
        canvas_set_empty_image();
        canvas_fill_rect(d->rect, d->fillColor);
    }
    if (d->borderColor.a > 0) {
        canvas_set_empty_image();
        canvas_stroke_rect(rect_expand(d->rect, 1.0f), d->borderColor, 1);
    }

    const char* cstr = text2d__c_str(d);
    const char* str = d->localized ? localize(cstr) : cstr;
    if (!str || *str == '\0') {
        return;
    }

    if (d->adjustsFontSizeToFitBounds) {
        adjust_font_size(str, d->rect);
    }

    get_text_block_size(str, text_block);

    const vec2_t position = add_vec2(
            d->rect.position,
            mul_vec2(
                    sub_vec2(
                            d->rect.size,
                            text_block->size
                    ),
                    d->format.alignment
            )
    );
    text_engine.position.x = position.x;
    text_engine.position.y = position.y + text_block->lines[0].ascender;
    draw_text_block(str, text_block);

    if (d->showTextBounds) {
        rect_t bounds;
        bounds.position = position;
        bounds.size = text_block->size;
        canvas_stroke_rect(rect_expand(bounds, 1.0f), RGB(0xFF0000), 1);
    }
}

rect_t text2d_get_bounds(entity_t e) {
    const text2d_t* d = get_text2d(e);
    if (d->hitTextBounds) {
        return text2d_get_text_bounds(d);
    }
    return d->rect;
}

rect_t text2d_get_text_bounds(const text2d_t* text) {
    text_block_t* text_block = &text_engine.text_block;
    text_engine.format = text->format;

    const char* cstr = text2d__c_str(text);
    const char* str = text->localized ? localize(cstr) : cstr;

    if (text->adjustsFontSizeToFitBounds) {
        adjust_font_size(str, text->rect);
    }

    get_text_block_size(str, text_block);
    const vec2_t textSize = text_block->size;

    rect_t textBounds;
    textBounds.position = add_vec2(
            text->rect.position,
            mul_vec2(
                    sub_vec2(
                            text->rect.size,
                            textSize
                    ),
                    text->format.alignment
            )
    );
    textBounds.size = textSize;
    return textBounds;
}

const char* text2d__c_str(const text2d_t* text2d) {
    switch (text2d->flags & TEXT2D_STR_MASK) {
        case TEXT2D_C_STR:
            return text2d->c_str;
        case TEXT2D_INPLACE:
            return text2d->buffer;
        case TEXT2D_STR_BUF:
            return str_get(text2d->str_obj);
    }
    // non-reachable
    return "";
}

void text2d__set_str_mode(text2d_t* text2d, uint32_t mode) {
    text2d->flags = ((text2d->flags >> 2u) << 2u) | mode;
}

bool text2d_hit_test(entity_t e, vec2_t point) {
    return rect_contains(text2d_get_bounds(e), point);
}

void set_text(entity_t e, const char* cstr) {
    text2d_t* txt = get_text2d(e);
    if (txt) {
        txt->c_str = cstr;
        text2d__set_str_mode(txt, 0);
    }
}

void set_text_timer(entity_t e, int millis, uint32_t flags) {
    text2d_t* txt = get_text2d(e);
    if (txt) {
        ek_cstr_format_timer(txt->buffer, sizeof txt->buffer, millis, flags);
        text2d__set_str_mode(txt, TEXT2D_INPLACE);
    }
}

void set_text_f(entity_t e, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);

    text2d_t* txt = get_text2d(e);
    if (txt) {
        ek_vsnprintf(txt->buffer, sizeof txt->buffer, fmt, va);
        text2d__set_str_mode(txt, TEXT2D_INPLACE);
    }

    va_end(va);
}

void copy_entity_text(entity_t to, entity_t from) {
    const text2d_t* src = get_text2d(from);
    text2d_t* dst = get_text2d(to);
    if (src && dst) {
        dst->flags = ((dst->flags >> 2u) << 2u) | (src->flags & TEXT2D_STR_MASK);
        dst->c_str = src->c_str;
        if (dst->str_obj != src->str_obj) {
            arr_assign_(&dst->str_obj, 1, src->str_obj);
        }
        memcpy(dst->buffer, src->buffer, sizeof src->buffer);
    }
}

text2d_t* text2d_setup(entity_t e) {
    display2d_t* display = add_display2d(e);
    text2d_t* drawable = add_text2d(e);
    display->draw = text2d_draw;
    display->hit_test = text2d_hit_test;
    display->get_bounds = text2d_get_bounds;
    return drawable;
}

text2d_t* text2d_setup_ex(entity_t e, text_format_t format) {
    text2d_t* d = text2d_setup(e);
    d->format = format;
    return d;
}

void set_text_dynamic(entity_t e, void* str_obj) {
    text2d_t* txt = get_text2d(e);
    if (txt) {
        if (txt->str_obj != str_obj) {
            str_copy(&txt->str_obj, str_obj);
        }
        text2d__set_str_mode(txt, TEXT2D_STR_BUF);
    }
}
