#ifndef EKX_APP_DISPLAY_H
#define EKX_APP_DISPLAY_H

#include <ek/gfx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct game_display_info {
    // when game drawing inside window viewport
    rect_t dest_viewport;
    vec2_t size;
    vec2_t window;
    vec4_t insets;
    vec4_t user_insets_abs;
    vec4_t user_insets_rel;
    float dpi_scale;
} game_display_info;

typedef struct game_display {
    game_display_info info;
    sg_pass pass;
    sg_image color;
    sg_image depth_stencil;
    void* screenshot_buffer;
    bool simulated;
    bool color_first_clear_flag;
} game_display;

void game_display_update(game_display* display);

bool game_display_begin(game_display* display, sg_pass_action* pass_action, const char* debug_label);
void game_display_end(game_display* display);

bool game_display_dev_begin(game_display* display);
void game_display_dev_end(game_display* display);

// implementation enabled only for EK_UITEST
void game_display_screenshot(const game_display* display, const char* filename);

// prints app's global display/window metrics: size and insets
void log_app_display_info(void);

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_DISPLAY_H
