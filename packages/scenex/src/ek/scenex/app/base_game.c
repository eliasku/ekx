#include "base_game.h"
#include "editor.h"

#include <ek/app.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/rnd.h>
#include <ek/audio.h>
#include <ek/gfx.h>
#include <ek/canvas.h>

#include <ek/scenex/assets/asset_manager.h>
#include <ekx/app/profiler.h>
#include <ek/scenex/text/text_engine.h>

#include <ekx/ng/updater.h>
#include <ekx/app/input_state.h>
/** resource managers include **/
#include <ek/scenex/particles/particle_decl.h>
#include <ek/scenex/scene_factory.h>
#include <ek/scenex/text/font.h>
#include <ek/scenex/2d/dynamic_atlas.h>
#include <ek/scenex/2d/atlas.h>

/** systems **/
#include "ek/scenex/base/node_events.h"
#include "ekx/app/uitest.h"
#include <ek/scenex/2d/viewport.h>
#include <ek/scenex/2d/layout_rect.h>
#include <ek/scenex/interaction_system.h>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/2d/camera2d.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/systems/main_flow.h>
#include <ek/scenex/2d/viewport.h>
#include <ek/scenex/2d/layout_rect.h>
#include <ek/scenex/systems/main_flow.h>
#include <ek/scenex/2d/camera2d.h>

#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/text2d.h>
#include <ek/scenex/base/tween.h>
#include <ek/goodies/camera_shaker.h>
#include <ek/goodies/bubble_text.h>
#include <ek/scenex/base/interactive.h>
#include <ek/goodies/trail2d.h>
#include <ek/scenex/base/node_events.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/particles/particle_system.h>
#include <ekx/ng/updater.h>

static void setup_scene_x(void) {
    //// basic scene
    setup_node();
    setup_transform2d();
    setup_display2d();
    setup_text2d();
    setup_camera2d();
    setup_movieclip();

    setup_layout_rect();
    setup_viewport();
    setup_button();
    setup_interactive();
    setup_node_events();

    setup_tween();
    setup_camera_shaker();
    setup_bubble_text();
    setup_trail2d();
    setup_particle2d();
    setup_updater();
}

game_app_state_t game_app_state;

void terminate_game_app(void) {
    if (game_app_state.delegate.terminate) {
        game_app_state.delegate.terminate();
    }
    game_app_state = (game_app_state_t)INIT_ZERO;
}

static void root_app_on_frame(void) {
    log_tick();
    ek_timers_update();
}

static void root_app_on_event(const ek_app_event ev) {
    if (ev.type == EK_APP_EVENT_PAUSE) {
        auph_set_pause(AUPH_MIXER, true);
    } else if (ev.type == EK_APP_EVENT_RESUME) {
        auph_set_pause(AUPH_MIXER, false);
    } else if (ev.type == EK_APP_EVENT_CLOSE) {
        ek_app.on_frame = NULL;
        ek_app.on_event = NULL;

        terminate_game_app();

        ecx_shutdown();
        canvas_shutdown();
        auph_shutdown();
        ek_gfx_shutdown();
    }
}

static float calc_preloader_progress(float init, float loading) {
    return 0.1f * init + 0.9f * loading;
}

static void draw_preloader(float progress, vec2_t size) {
    canvas_set_empty_image();
    const float pad = 40.0f;
    const float w = size.x - pad * 2.0f;
    const float h = 16.0f;
    const float y = (size.y - h) / 2.0f;

    canvas_quad_color(pad, y, w, h, COLOR_WHITE);
    canvas_quad_color(pad + 2, y + 2, w - 4, h - 4, COLOR_BLACK);
    canvas_quad_color(pad + 4, y + 4, (w - 8) * progress, h - 8, COLOR_WHITE);

    const float sz = size.x < size.y ? size.x : size.y;
    const float cx = size.x / 2.0f;
    const float cy = size.y / 2.0f;
    const float sh = sz / 16.0f;
    const float sw = sh * 3;
    const float time = (float) ek_time_now();
    for (int i = 0; i < 7; ++i) {
        float r = ((float) i / 7) * 1.5f + time;
        float speed = (0.5f + 0.5f * sinf(r * 2 + 1));
        r = r + 0.5f * speed;
        float ox = sinf(r * 3.14f);
        float oy = sinf(r * 3.14f * 2 + 3.14f);
        float R = (sh / 10.0f) *
                  (1.8f - 0.33f * speed - 0.33f * ((cosf(r * 3.14f) + 2.0f * cosf(r * 3.14f * 2 + 3.14f))));
        canvas_fill_circle(vec3(cx + ox * sw, cy - 2.0f * sh + oy * sh, R), COLOR_WHITE, COLOR_WHITE, 16);
    }
}

static sg_color sg_color_rgb(uint32_t rgb) {
    union {
        vec4_t vec;
        sg_color color;
    } c = {vec4_color(RGB(rgb))};
    return c.color;
}

static void game_app_pre_load(void) {
    log_debug("game_app: pre load");

    profiler_init();
    init_time_layers();
    log_app_display_info();
    game_display_update(&game_app_state.display);

    log_debug("base application: initialize scene root");
    entity_t root = create_node2d(H("root"));
    game_app_state.root = root;

    viewport_t* vp = add_viewport(root);
    const vec2_t resolution = vec2(ek_app.config.width, ek_app.config.height);
    vp->options.baseResolution = resolution;
    LayoutRect_design_canvas_rect = rect_size(resolution);

    add_layout_rect(root);
    add_node_events(root);
    Viewport_update(&game_app_state.display.info);
    game_app_state.scale_factor = vp->output.scale;
    assets_set_scale_factor(vp->output.scale);

    log_debug("base application: initialize InteractionSystem");
    g_interaction_system.root_ = root;
    log_debug("base application: initialize AudioManager");
    init_audio_manager();

    log_debug("base application: initialize Scene");
    main_camera = create_node2d(H("camera"));
    camera2d_t* default_camera = add_camera2d(main_camera);
    default_camera->root = root;
    default_camera->viewportNode = root;
    append(root, main_camera);
}

static void game_app_update_frame(float dt) {
    entity_t root = game_app_state.root;
    EDITOR_UPDATE;
    scene_pre_update(root, dt);
    if(game_app_state.delegate.update) {
        game_app_state.delegate.update();
    }
    scene_post_update(root);
}

static void game_app_frame(void) {
    uint64_t timer = ek_ticks(NULL);

    game_display* display = &game_app_state.display;
    entity_t root = game_app_state.root;
    root_app_on_frame();

    EDITOR_BEFORE_FRAME_BEGIN;
    game_display_update(display);
    Viewport_update(&display->info);
    float scale_factor = get_viewport(root)->output.scale;
    game_app_state.scale_factor = scale_factor;
    assets_set_scale_factor(scale_factor);

    vec2_t d_size = display->info.size;

    /** base app BEGIN **/

    const float dt = fminf((float) update_frame_timer(&game_app_state.frame_timer), 0.3f);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;
    game_app_update_frame(dt);

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_UPDATE, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

    canvas_new_frame();

    /// PRE-RENDER
    if(game_app_state.delegate.pre_render) {
        game_app_state.delegate.pre_render();
    }
    EDITOR_PRE_RENDER;

    if (!game_app_state.started) {
        assets_root_pack_poll();
    }

    sg_pass_action pass_action = INIT_ZERO;
    pass_action.colors[0].action = game_app_state.started ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR;
    pass_action.colors[0].value = sg_color_rgb(ek_app.config.background_color);
    if (ek_app.config.need_depth) {
        pass_action.depth.action = SG_ACTION_CLEAR;
        pass_action.depth.value = 1.0f;
    }
    if (game_display_begin(display, &pass_action, "game-view")) {
        profiler_render_begin(d_size.x * d_size.y);

        {
            /*** SCENE DRAWING ***/
            if (game_app_state.delegate.pre_render_scene) {
                game_app_state.delegate.pre_render_scene();
            }
            scene_render(game_app_state.root);
            if (game_app_state.delegate.post_render_scene) {
                game_app_state.delegate.post_render_scene();
            }
        }

        canvas_begin(d_size.x, d_size.y);

        if (!game_app_state.started) {
            const float progress = assets_root_pack_progress();
            draw_preloader(calc_preloader_progress(1, progress), d_size);
        }

        elapsed = ek_ticks_to_sec(ek_ticks(&timer));
        profiler_add_time(PROFILE_RENDER, (float) (elapsed * 1000));
        profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

        profiler_render_end();
        profiler_draw(&display->info);

        canvas_end();

        game_display_end(display);

        if (game_display_dev_begin(display)) {

            EDITOR_RENDER_OVERLAY;

            // TODO: do we need this step?
            //canvas_begin(ek_app.viewport.width, ek_app.viewport.height);
            //g_game_app->onFrameEnd();
            //canvas_end();

            elapsed = ek_ticks_to_sec(ek_ticks(&timer));
            profiler_add_time(PROFILE_OVERLAY, (float) (elapsed * 1000));
            profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

            game_display_dev_end(display);
        }
    }
    sg_commit();

    if (!game_app_state.started && assets_is_all_loaded()) {
        log_debug("Start Game");
        if(game_app_state.delegate.start) {
            game_app_state.delegate.start();
        }
        game_app_state.started = true;

        UITEST_START;
    }

    input_state_post_update();
    EDITOR_POST_FRAME;

    elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_END, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

    profiler_update((float) game_app_state.frame_timer.dt);
}

static void game_app_event(const ek_app_event event) {
    uint64_t timer = ek_ticks(NULL);

    input_state_process_event(&event, &game_app_state.display.info);

    EDITOR_EVENT(event);
    if(game_app_state.delegate.event) {
        game_app_state.delegate.event(event);
    }

    root_app_on_event(event);
    if (event.type == EK_APP_EVENT_RESIZE) {
        game_display_update(&game_app_state.display);
    }

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_EVENTS, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));
}

// Game app activation

static void launcher_on_ready(void) {
}

static void launcher_on_frame(void) {
    root_app_on_frame();

    static int init_step = 0;

    const float steps = 5.0f;

    //EK_PROFILE_SCOPE(INIT_JOB);
    switch (init_step) {
        case 0:
            ++init_step;
            ek_gfx_setup(DRAW_CALLS_MAX);
            EDITOR_SETUP;
            break;
        case 1:
            ++init_step;
            canvas_setup();
            break;
        case 2:
            ++init_step;
            ecx_setup();
            setup_scene_x();
            break;
        case 3:
            ++init_step;
            UITEST_SETUP;
            game_app_pre_load();
            break;
        case 4:
            // dispatch pre-load event
            if(game_app_state.delegate.pre_load) {
                game_app_state.delegate.pre_load();
            }
            // load root pack asset, which will load all dependent assets
            assets_root_pack_load("pack.bin");
            ++init_step;
            break;
        default:
            ek_app.on_frame = game_app_frame;
            ek_app.on_event = game_app_event;
            break;
    }

    if (init_step > 0) {
        const float width = ek_app.viewport.width;
        const float height = ek_app.viewport.height;
        if (width > 0 && height > 0) {
            //EK_PROFILE_SCOPE("init frame");
            sg_pass_action pass_action = INIT_ZERO;
            pass_action.colors[0].action = SG_ACTION_CLEAR;
            pass_action.colors[0].value = sg_color_rgb(ek_app.config.background_color);
            sg_begin_default_pass(&pass_action, (int) width, (int) height);

            if (init_step > 2) {
                canvas_new_frame();
                canvas_begin(width, height);
                draw_preloader(calc_preloader_progress((float) init_step / steps, 0), vec2(width, height));
                canvas_end();
            }

            sg_end_pass();
            sg_commit();
        }
    }
}

static void setup_resource_managers(void) {
    setup_res_sprite();
    setup_res_particle();
    setup_res_dynamic_atlas();
    setup_res_atlas();
    setup_res_font();
    setup_res_sg();
}

void run_app(void) {

    log_init();
    ek_time_init();
    ek_timers_init();

    uint32_t seed = ek_time_seed32();
    random_seed = seed++;
    game_random_seed = seed;

    // audio should be initialized before "Resume" event, so the best place is "On Create" event
    audio_setup();

    // setup resources before app constructor, for example because Profiler requires font id resolving
    setup_resource_managers();

    setup_text_engine();

    assets_init();
    game_app_state.scale_factor = 1;

    EDITOR_INIT;

    ek_app.on_ready = launcher_on_ready;
    ek_app.on_frame = launcher_on_frame;
    ek_app.on_event = root_app_on_event;
}

