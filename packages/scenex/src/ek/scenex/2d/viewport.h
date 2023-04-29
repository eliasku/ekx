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

#define Viewport ECX_ID(viewport_t)

extern ECX_DEFINE_TYPE(viewport_t);
void setup_viewport(void);
#define get_viewport(e) ECX_GET(viewport_t,e)
#define add_viewport(e) ECX_ADD(viewport_t,e)

#ifdef __cplusplus
}
#endif

#endif // SCENEX_VIEWPORT_H
