#pragma once

#include <ek/math.h>
#include <ecx/ecx_fwd.hpp>
#include <ekx/app/game_display.h>

namespace ek {

/*** Scale Camera viewport to Screen ***/
struct ViewportScaleOptions {
    // logical resolution size,
    vec2_t baseResolution = vec2(1, 1);
    vec2_t alignment = vec2(0.5f, 0.5f);
    vec2_t safeAreaFit = vec2(1, 1);
    // modes
    vec2_t pixelRatio = vec2(1, 1);
    rect_t viewport = rect_01();
    bool scaleToResolution = true;
};

struct ViewportScaleInput {
    // screen metrics
    rect_t fullRect;
    rect_t safeRect;
    float dpiScale;
};

// NOTE: we need default initialization because could be not updated after recreating viewports on camera processing
struct ViewportScaleOutput {
    rect_t screenRect = rect_01();
    rect_t fullRect = rect_01();
    rect_t safeRect = rect_01();
    vec2_t offset = {};
    float scale = 1.0f;
};

struct Viewport {
    ViewportScaleOptions options{};
    ViewportScaleOutput output{};

//    Viewport() = default;

    static void updateAll(const game_display_info* display0);
};

}