#ifndef SCENEX_TEXT2D_H
#define SCENEX_TEXT2D_H

#include <ek/scenex/text/text_format.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TEXT2D_C_STR = 0,
    TEXT2D_INPLACE = 1,
    TEXT2D_STR_BUF = 2,
    TEXT2D_STR_MASK = 0x3,
    TEXT2D_LOCALIZED = 4,
} text2d_flags;


// 8 + 136 + 16 + 8 + 4 = 172 bytes
typedef struct {
    // 3 modes for string storage:
    // 0 - const c-string
    // 1 - inplace buffer
    // 2 - dynamic string buffer
    char buffer[32];
    void* str_obj;
    const char* c_str;

    uint32_t flags;

    text_format_t format;
    rect_t rect;

    color_t borderColor;
    color_t fillColor;

    bool localized;

    // Reduce font-size until text fits to the field bounds (if bounds not empty)
    bool adjustsFontSizeToFitBounds;

    // if enabled hit test visible glyphs bounds, otherwise hits whole text field rect area
    bool hitTextBounds;

    // debug metrics
    bool showTextBounds;
} text2d_t;

rect_t text2d_get_text_bounds(const text2d_t* text);

void set_text_dynamic(entity_t e, void* str_obj);

text2d_t* text2d_setup_ex(entity_t e, text_format_t format);

const char* text2d__c_str(const text2d_t* text2d);

void text2d__set_str_mode(text2d_t* text2d, uint32_t mode);

void set_text_f(entity_t e, const char* fmt, ...);

void copy_entity_text(entity_t to, entity_t from);

void set_text(entity_t e, const char* cstr);

void set_text_timer(entity_t e, int millis, uint32_t flags);

// text2d setup
text2d_t* text2d_setup(entity_t e);

void text2d_draw(entity_t e);

rect_t text2d_get_bounds(entity_t e);

bool text2d_hit_test(entity_t e, vec2_t lp);

extern ecx_component_type Text2D;
void Text2D_setup(void);
#define get_text2d(e) ((text2d_t*)get_component(&Text2D, e))
#define add_text2d(e) ((text2d_t*)add_component(&Text2D, e))

#ifdef __cplusplus
}
#endif

#endif // SCENEX_TEXT2D_H
