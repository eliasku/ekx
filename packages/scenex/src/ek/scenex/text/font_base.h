#ifndef SCENEX_FONT_BASE_H
#define SCENEX_FONT_BASE_H

#include <ek/math.h>
#include <sokol/sokol_gfx.h>
#include "font_type.h"

struct font_base_ {
    explicit font_base_(font_type_t type);

    virtual ~font_base_() = 0;

    virtual bool getGlyph(uint32_t codepoint, glyph_t* outGlyph) = 0;

    virtual bool getGlyphMetrics(uint32_t codepoint, glyph_t* outGlyph) = 0;

    virtual float getKerning(uint32_t codepoint1, uint32_t codepoint2) = 0;

    virtual void setBlur(float radius, int iterations, int strengthPower) {
        (void)radius;
        (void)iterations;
        (void)strengthPower;
        // by default blur is not supported
    }

    [[nodiscard]] font_type_t getFontType() const {
        return fontType;
    }

    bool isReady() const;

protected:
    font_type_t fontType;
    float lineHeightMultiplier;

    bool loaded_ = false;
    bool ready_ = false;
};

#endif // SCENEX_FONT_BASE_H
