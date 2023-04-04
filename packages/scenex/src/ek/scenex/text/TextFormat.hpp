#pragma once

#include <ek/math.h>
#include <ek/math.h>

#include "Font.hpp"

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

inline vec2_t vec2_ui_align(alignment_t alignment, vec2_t v) {
    return vec2(
            (alignment & ALIGNMENT_RIGHT) ? 1.0f : ((alignment & ALIGNMENT_LEFT) ? 0.0f : (alignment & ALIGNMENT_CENTER_X ? 0.5f : v.x)),
            (alignment & ALIGNMENT_BOTTOM) ? 1.0f : ((alignment & ALIGNMENT_TOP) ? 0.0f : (alignment & ALIGNMENT_CENTER_Y ? 0.5f : v.y))
    );
}

namespace ek {

struct Font;

enum class TextLayerType {
    Text,
    Stroke1,
    Stroke2,
    Shadow
};

// type + color + offset = 16
// blur + effects = 8
// Total: 24 bytes
struct TextLayerEffect {
    TextLayerType type = TextLayerType::Text;
    color_t color = COLOR_WHITE;
    vec2_t offset = {};
    float blurRadius = 0.0f;
    uint8_t blurIterations = 0;
    uint8_t strength = 0;
    bool visible = true;
    bool showGlyphBounds = false;
};

// 8 + 4 + 4 + 4 + 8 + 4(flags) = 32
// layers = 24 bytes * 4 (LayersMax) = 96
// count / indx = 8
// Total = 136 bytes
struct TextFormat {
    R(Font) font = 0;
    float size = 16.0f;
    float leading = 0.0f;
    float letterSpacing = 0.0f;

    // not actually box alignment , but just to be shared for single line
    vec2_t alignment = vec2(0,0);

    bool kerning = true;
    bool underline = false;
    bool strikethrough = false;
    bool wordWrap = false;
    bool allowLetterWrap = true;

    inline static const int LayersMax = 4;

    TextLayerEffect layers[LayersMax]{};
    int layersCount = 1;
    int textLayerIndex = 0;

    void setTextColor(color_t color) {
        layers[textLayerIndex].color = color;
    }

    [[nodiscard]] color_t getTextColor() const {
        return layers[textLayerIndex].color;
    }

    void setFontName(string_hash_t fontName) {
        font = R_FONT(fontName);
    }

    TextFormat() = default;

    TextFormat(string_hash_t fontName, float fontSize) :
            font{R_FONT(fontName)},
            size{fontSize} {
    }

    void setAlignment(alignment_t align) {
        alignment = vec2_ui_align(align, vec2(0.5f, 0.5f));
    }

    void addShadow(color_t color, float radius, vec2_t offset = {}, float hardness = 0.2f /* 0..1 */) {
        auto& layer = layers[layersCount++];
        layer.type = TextLayerType::Shadow;
        layer.color = color;
        layer.strength = (uint8_t)(7 * hardness);
        layer.blurIterations = 3;
        layer.blurRadius = radius;
        layer.offset = offset;
    }
};

}