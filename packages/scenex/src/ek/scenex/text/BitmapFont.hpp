#ifndef SCENEX_BITMAP_FONT_H
#define SCENEX_BITMAP_FONT_H

#include "font_base.h"
#include <gen_sg.h>
#include <ek/ds/Hash.hpp>

class BitmapFont : public font_base_ {
public:

    BitmapFont();

    ~BitmapFont() override;

    void load(const uint8_t* buffer, size_t length);

    bool getGlyph(uint32_t codepoint, glyph_t* outGlyph) override;

    bool getGlyphMetrics(uint32_t codepoint, glyph_t* outGlyph) override;

    float getKerning(uint32_t codepoint1, uint32_t codepoint2) override;

public:
    // bmfont is mapped to memory, we use this control structure with mapped pointers as source of data
    bmfont_t file = {};
    // dynamic hash-map to map 32-bit codepoint to 32-bit glyph index
    ek::Hash<uint32_t> map;
};

#endif // SCENEX_BITMAP_FONT_H
