#include "font_base.h"

font_base_::font_base_(font_type_t fontType_) :
        fontType{fontType_},
        lineHeightMultiplier{1.f} {

}

font_base_::~font_base_() = default;

bool font_base_::isReady() const {
    return ready_ && loaded_;
}

