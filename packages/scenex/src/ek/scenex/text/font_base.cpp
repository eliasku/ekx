#include "font_base.h"

font_base_::font_base_(font_type_t fontType_) :
        fontType{fontType_},
        lineHeightMultiplier{1.f} {

}

font_base_::~font_base_() = default;

bool font_base_::isReady() const {
    return ready_ && loaded_;
}

float font_base_kerning(font_base_t* font, uint32_t codepoint1, uint32_t codepoint2) {
    return font->getKerning(codepoint1, codepoint2);
}

