#include "Viewport.hpp"
#include "LayoutRect.hpp"
#include <ekx/app/game_display.h>

namespace ek {

void doScale(const ViewportScaleInput& input, const ViewportScaleOptions& options, ViewportScaleOutput& output) {
    auto fullRect = input.fullRect;
    fullRect.position += options.viewport.position * fullRect.size;
    fullRect.size = fullRect.size * options.viewport.size;
    const auto safeRect = rect_clamp_bounds(input.safeRect, fullRect);

    if (options.scaleToResolution) {
        // interpolate between full and safe area
        rect_t rc;
        rc.position = fullRect.position + (safeRect.position - fullRect.position) * options.safeAreaFit;
        rc.size = fullRect.size + (safeRect.size - fullRect.size) * options.safeAreaFit;
        const float scale = fminf(rc.w / options.baseResolution.x, rc.h / options.baseResolution.y);
        output.offset = options.alignment * (rc.size - scale * options.baseResolution);
        output.scale = scale;
    } else {
        output.scale = length_vec2(options.pixelRatio);
        rect_t rc;
        rc.position = fullRect.position + (safeRect.position - fullRect.position) * options.safeAreaFit;
        rc.size = fullRect.size + (safeRect.size - fullRect.size) * options.safeAreaFit;
        output.offset = options.alignment * (rc.size - output.scale * options.baseResolution);
    }

    output.offset += safeRect.position * options.safeAreaFit;

    const auto invScale = 1.0f / output.scale;
    output.fullRect = rect_scale_f(rect_translate(fullRect, -output.offset), invScale);
    output.safeRect = rect_scale_f(rect_translate(safeRect, -output.offset), invScale);

    output.screenRect.position = input.fullRect.position + options.viewport.position * input.fullRect.size;
    output.screenRect.size = options.viewport.size * input.fullRect.size;
}

void updateViewport(entity_t e, const ViewportScaleInput& input) {
    auto& vp = ecs::get<Viewport>(e);

    doScale(input, vp.options, vp.output);

    auto& layout = ecs::get<LayoutRect>(e);
    layout.rect = vp.output.fullRect;
    layout.safeRect = vp.output.safeRect;
}

void Viewport::updateAll(const game_display_info* display0) {
    const float w = display0->size.x;
    const float h = display0->size.y;
    const vec4_t insets =
            display0->insets +
            display0->userInsetsAbsolute +
            vec4(w, h, w, h) * display0->userInsetsRelative;
    ViewportScaleInput input;
    input.fullRect = rect_wh(display0->size.x, display0->size.y);
    input.safeRect = rect(insets.x, insets.y, w - insets.x - insets.z, h - insets.y - insets.w);
    input.dpiScale = display0->dpiScale;

    for (auto e: ecs::view<Viewport>()) {
        updateViewport(e, input);
    }
}

}
