#include "text_format.h"
#include "font.h"

vec2_t vec2_ui_align(alignment_t alignment, vec2_t v) {
    return vec2(
            (alignment & ALIGNMENT_RIGHT) ? 1.0f : ((alignment & ALIGNMENT_LEFT) ? 0.0f : (alignment &
                                                                                           ALIGNMENT_CENTER_X ? 0.5f
                                                                                                              : v.x)),
            (alignment & ALIGNMENT_BOTTOM) ? 1.0f : ((alignment & ALIGNMENT_TOP) ? 0.0f : (alignment &
                                                                                           ALIGNMENT_CENTER_Y ? 0.5f
                                                                                                              : v.y))
    );
}

void
text_format_set_text_color(text_format_t* tf, color_t color) {
    EK_ASSERT(tf->textLayerIndex < tf->layersCount);
    tf->layers[tf->textLayerIndex].color = color;
}

color_t
text_format_get_text_color(const text_format_t* tf) {
    EK_ASSERT(tf->textLayerIndex < tf->layersCount);
    return tf->layers[tf->textLayerIndex].color;
}

void
text_format_set_font(text_format_t* tf, string_hash_t font_name) {
    tf->font = R_FONT(font_name);
}

void
text_format_align(text_format_t* tf, alignment_t align) {
    tf->alignment = vec2_ui_align(align, vec2(0.5f, 0.5f));
}

// hardness = 0.2f /* 0..1 */
// offset = vec2(0, 0)
void
text_format_add_shadow(text_format_t* tf, color_t color, float radius, vec2_t offset, float hardness) {
    text_layer_effect_t layer = INIT_ZERO;
    layer.type = TEXT_LAYER_SHADOW;
    layer.color = color;
    layer.offset = offset;
    layer.blurRadius = radius;
    layer.blurIterations = 3;
    layer.strength = (uint8_t)(7 * hardness);
    layer.visible = true;
    EK_ASSERT(tf->layersCount < TEXT_LAYERS_MAX);
    tf->layers[tf->layersCount++] = layer;
}

text_format_t
text_format(string_hash_t font_name, float size) {
    text_format_t f = INIT_ZERO;
    f.layers[0].color = RGB(0xFFFFFF);
    f.layers[0].visible = true;
    f.layersCount = 1;
    f.kerning = true;
    f.allowLetterWrap = true;
    f.font = R_FONT(font_name);
    f.size = size;
    return f;
}

