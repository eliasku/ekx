#include <ekx/app/game_display.h>

#include <ek/canvas.h>
#include <ek/app.h>
#include <ek/log.h>

#ifdef EK_UITEST
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif

bool game_display_begin(game_display* display, sg_pass_action* pass_action, const char* debug_label) {
    const int w = (int) display->info.size.x;
    const int h = (int) display->info.size.y;
    if (w <= 0 || h <= 0) {
        return false;
    }
    sg_push_debug_group(debug_label);

    if (display->simulated) {
        canvas.framebuffer_color = display->color;
        canvas.framebuffer_depth = display->depth_stencil;

        if (display->color_first_clear_flag) {
            pass_action->colors[0].action = SG_ACTION_CLEAR;
            display->color_first_clear_flag = false;
        }
        sg_begin_pass(display->pass, pass_action);
    } else {
        sg_begin_default_pass(pass_action, w, h);
    }

    return true;
}

void game_display_end(game_display* display) {
    if (display->simulated) {
        sg_end_pass();
        canvas.framebuffer_color.id = SG_INVALID_ID;
        canvas.framebuffer_depth.id = SG_INVALID_ID;
    }
    sg_pop_debug_group();
}

static void game_display_debug_sim_fullscreen(game_display* display) {
    // draw offscreen
    sg_push_debug_group("debug fullscreen ");

    const float fw = ek_app.viewport.width;
    const float fh = ek_app.viewport.height;
    const float scalex = fw / display->info.size.x;
    const float scaley = fh / display->info.size.y;
    const float scale = MIN(scalex, scaley);
    canvas_begin(fw, fh);
    canvas_set_image(display->color);
    canvas_set_image_rect(rect_01());
    canvas_quad(0, 0, scale * display->info.size.x, scale * display->info.size.y);
    canvas_end();

    sg_pop_debug_group();
}

bool game_display_dev_begin(game_display* display) {
    const float fw = ek_app.viewport.width;
    const float fh = ek_app.viewport.height;
    const int w = (int) fw;
    const int h = (int) fh;
    if (w <= 0 || h <= 0) {
        return false;
    }

    if (display->simulated) {
        sg_pass_action pass_action = {0};
        pass_action.colors[0].action = SG_ACTION_DONTCARE;
        sg_begin_default_pass(&pass_action, w, h);

        // TODO: move this call to in-game dev tools
        game_display_debug_sim_fullscreen(display);
    }

    return true;
}

void game_display_dev_end(game_display* display) {
    (void) display;
    sg_end_pass();
//    sg_commit();
//    if (simulated) {
//        screenshot("screenshot.png");
//    }
}

sg_image game_display_create_image(int w, int h, bool is_color, const char* label) {
    const sg_image_desc desc = {
        .type = SG_IMAGETYPE_2D,
        .render_target = true,
        .width = w,
        .height = h,
        .usage = SG_USAGE_IMMUTABLE,
        .pixel_format = is_color ? 0 : SG_PIXELFORMAT_DEPTH_STENCIL,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .label = label,
    };
    return sg_make_image(&desc);
}

void game_display_update_simulated(game_display* display) {
    // limit min size
    display->info.size.x = fmaxf(16.0f, display->info.size.x);
    display->info.size.y = fmaxf(16.0f, display->info.size.y);
    display->info.window.x = display->info.size.x;
    display->info.window.y = display->info.size.y;

    const int w = (int) display->info.size.x;
    const int h = (int) display->info.size.y;

    int color_img_width = 0;
    int color_img_height = 0;
    if (display->color.id) {
        const sg_image_desc color_image_info = sg_query_image_desc(display->color);
        color_img_width = color_image_info.width;
        color_img_height = color_image_info.height;
    }
    // re-create
    if (display->color.id == 0 || color_img_width != w || color_img_height != h) {
        sg_destroy_image(display->color);
        display->color = game_display_create_image(w, h, true, "game-display-color");
        color_img_width = w;
        color_img_height = h;

        display->color_first_clear_flag = true;

        if (ek_app.config.flags & EK_APP_CONFIG_DEPTH) {
            sg_destroy_image(display->depth_stencil);
            display->depth_stencil = game_display_create_image(w, h, false, "game-display-depth");
        }

        sg_destroy_pass(display->pass);
        sg_pass_desc pass_desc = {0};
        pass_desc.color_attachments[0].image = display->color;
        if (display->depth_stencil.id) {
            pass_desc.depth_stencil_attachment.image = display->depth_stencil;
        }
        display->pass = sg_make_pass(&pass_desc);

        free(display->screenshot_buffer);
        display->screenshot_buffer = malloc(w * h * 4);
    }
}

void game_display_update(game_display* display) {
    if (display->simulated) {
        game_display_update_simulated(display);
        return;
    }
    display->info.size.x = ek_app.viewport.width;
    display->info.size.y = ek_app.viewport.height;
    display->info.window.x = ek_app.viewport.width / ek_app.viewport.scale;
    display->info.window.y = ek_app.viewport.height / ek_app.viewport.scale;
    display->info.insets = *(vec4_t*) ek_app.viewport.insets;
    display->info.dpi_scale = ek_app.viewport.scale;

    display->info.dest_viewport.position = vec2(0, 0);
    display->info.dest_viewport.size.x = ek_app.viewport.width;
    display->info.dest_viewport.size.y = ek_app.viewport.height;
}

void game_display_screenshot(const game_display* display, const char* filename) {
#ifdef EK_UITEST
    if (display->simulated && display->screenshot_buffer) {
        // TODO: get w/h
        const sg_image_desc desc = sg_query_image_desc(display->color);
        const int wi = desc.width;
        const int he = desc.height;
        if(ek_gfx_read_pixels(display->color, display->screenshot_buffer)) {
            stbi_write_png(filename, wi, he, 4, display->screenshot_buffer, 4 * wi);
        }
    }
#else
    UNUSED(display);
    UNUSED(filename);
#endif
}

void log_app_display_info(void) {
#ifndef NDEBUG
    log_info("Display: %d x %d", (int) ek_app.viewport.width, (int) ek_app.viewport.height);
    log_info("Insets: %d, %d, %d, %d",
             (int) ek_app.viewport.insets[0],
             (int) ek_app.viewport.insets[1],
             (int) ek_app.viewport.insets[2],
             (int) ek_app.viewport.insets[3]);
#endif
}
