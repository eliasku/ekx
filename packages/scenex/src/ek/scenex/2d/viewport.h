#ifndef SCENEX_VIEWPORT_H
#define SCENEX_VIEWPORT_H

#include <ek/math.h>
#include <ecx/ecx.h>
#include <ekx/app/game_display.h>

#ifdef __cplusplus
extern "C" {
#endif

/*** Scale Camera viewport to Screen ***/
typedef struct {
    // logical resolution size,
    vec2_t baseResolution;
    vec2_t alignment;
    vec2_t safeAreaFit;
    // modes
    vec2_t pixelRatio;
    rect_t viewport;
    bool scaleToResolution;
} viewport_scale_options_t;

typedef struct {
    // screen metrics
    rect_t fullRect;
    rect_t safeRect;
    float dpiScale;
}  viewport_scale_input_t;

// NOTE: we need default initialization because could be not updated after recreating viewports on camera processing
typedef struct {
    rect_t screenRect;
    rect_t fullRect;
    rect_t safeRect;
    vec2_t offset;
    float scale;
} viewport_scale_output_t;

typedef struct {
    viewport_scale_options_t options;
    viewport_scale_output_t output;
} viewport_t;

void Viewport_update(const game_display_info* display0);

extern ecx_component_type Viewport;
void Viewport_setup(void);
#define get_viewport(e) ((viewport_t*)get_component(&Viewport, e))
#define add_viewport(e) ((viewport_t*)add_component(&Viewport, e))

#ifdef __cplusplus
}
#endif

#endif // SCENEX_VIEWPORT_H
