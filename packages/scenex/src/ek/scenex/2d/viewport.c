#include "viewport.h"
#include "layout_rect.h"
#include <ekx/app/game_display.h>

ECX_DEFINE_TYPE(viewport_t);

static viewport_scale_options_t viewport_scale_options(void) {
    return (viewport_scale_options_t) {
            .baseResolution = vec2(1, 1),
            .alignment = vec2(0.5f, 0.5f),
            .safeAreaFit = vec2(1, 1),
            .pixelRatio = vec2(1, 1),
            .viewport = rect_01(),
            .scaleToResolution = true,
    };
}

static viewport_scale_output_t viewport_scale_output(void) {
    return (viewport_scale_output_t) {
            .screenRect = rect_01(),
            .fullRect = rect_01(),
            .safeRect = rect_01(),
            .offset = vec2(0, 0),
            .scale = 1.0f,
    };
}

static void viewport_ctor(component_handle_t i) {
    ((viewport_t*) Viewport.data[0])[i] = (viewport_t) {
            viewport_scale_options(),
            viewport_scale_output(),
    };
}

void setup_viewport(void) {
    ECX_TYPE(viewport_t, 4);
    Viewport.ctor = viewport_ctor;
}

static void do_scale(const viewport_scale_input_t* input, const viewport_scale_options_t* options,
                     viewport_scale_output_t* output) {
    rect_t fullRect = input->fullRect;
    fullRect.position = add_vec2(fullRect.position, mul_vec2(options->viewport.position, fullRect.size));
    fullRect.size = mul_vec2(fullRect.size, options->viewport.size);
    const rect_t safeRect = rect_clamp_bounds(input->safeRect, fullRect);

    if (options->scaleToResolution) {
        // interpolate between full and safe area
        rect_t rc;
        rc.position = add_vec2(fullRect.position,
                               mul_vec2(sub_vec2(safeRect.position, fullRect.position), options->safeAreaFit));
        rc.size = add_vec2(fullRect.size, mul_vec2(sub_vec2(safeRect.size, fullRect.size), options->safeAreaFit));
        const float scale = fminf(rc.w / options->baseResolution.x, rc.h / options->baseResolution.y);
        output->offset = mul_vec2(options->alignment, sub_vec2(rc.size, scale_vec2(options->baseResolution, scale)));
        output->scale = scale;
    } else {
        output->scale = length_vec2(options->pixelRatio);
        rect_t rc;
        rc.position = add_vec2(
                fullRect.position,
                mul_vec2(
                        sub_vec2(safeRect.position, fullRect.position),
                        options->safeAreaFit
                )
        );
        rc.size = add_vec2(fullRect.size, mul_vec2(sub_vec2(safeRect.size, fullRect.size), options->safeAreaFit));
        output->offset = mul_vec2(options->alignment,
                                  sub_vec2(rc.size, scale_vec2(options->baseResolution, output->scale)));
    }

    output->offset = add_vec2(output->offset, mul_vec2(safeRect.position, options->safeAreaFit));

    const float invScale = 1.0f / output->scale;
    output->fullRect = rect_scale_f(rect_translate(fullRect, neg_vec2(output->offset)), invScale);
    output->safeRect = rect_scale_f(rect_translate(safeRect, neg_vec2(output->offset)), invScale);
    output->screenRect.position = add_vec2(
            input->fullRect.position,
            mul_vec2(options->viewport.position, input->fullRect.size)
    );
    output->screenRect.size = mul_vec2(options->viewport.size, input->fullRect.size);
}

static void update_viewport(entity_t e, const viewport_scale_input_t* input) {
    viewport_t* vp = get_viewport(e);

    do_scale(input, &vp->options, &vp->output);

    layout_rect_t* layout = get_layout_rect(e);
    layout->rect = vp->output.fullRect;
    layout->safeRect = vp->output.safeRect;
}

void Viewport_update(const game_display_info* display0) {
    const float w = display0->size.x;
    const float h = display0->size.y;
    const vec4_t insets =
            add_vec4(display0->insets,
                     add_vec4(
                             display0->user_insets_abs,
                             mul_vec4(
                                     vec4(w, h, w, h),
                                     display0->user_insets_rel
                             )
                     )
            );
    viewport_scale_input_t input;
    input.fullRect = rect_wh(display0->size.x, display0->size.y);
    input.safeRect = rect(insets.x, insets.y, w - insets.x - insets.z, h - insets.y - insets.w);
    input.dpiScale = display0->dpi_scale;

    for (uint32_t i = 1; i < Viewport.size; ++i) {
        entity_t e = get_entity(&Viewport, i);
        update_viewport(e, &input);
    }
}

