#ifndef SCENEX_BASIC_APPLICATION_H
#define SCENEX_BASIC_APPLICATION_H

#include <ecx/ecx_fwd.hpp>

#ifdef EK_DEV_TOOLS

#include <ek/editor/Editor.hpp>

#endif

#include <ek/app.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/rnd.h>
#include <ek/audio.h>

#include <utility>
#include <ekx/app/profiler.h>
#include <ekx/app/game_display.h>
#include <ekx/app/frame_timer.h>
#include <ek/scenex/text/text_engine.h>
#include <ek/scenex/asset2/Asset.hpp>
#include "GameAppDispatcher.hpp"
#include "root_app_callbacks.h"

namespace ek {
class basic_application;
}

extern ek::basic_application* g_game_app;

void init_game_app(ek::basic_application* game);

struct Asset;

namespace ek {

void basic_app_on_frame();

void basic_app_on_event(ek_app_event);

class basic_application {
public:
    game_display display = {};
    frame_timer_t frame_timer = {};

/**** assets ***/
    float scale_factor = 1.0f;

    GameAppDispatcher dispatcher{};

    /////
    entity_t root;

    basic_application();

    virtual ~basic_application();

    virtual void initialize();

    virtual void terminate();

    virtual void preload();

    void onFrame();

    void onEvent(ek_app_event event);

public:
    bool preloadOnStart = true;
    Asset* rootAssetObject = nullptr;

protected:
    void doUpdateFrame(float dt);

    void doRenderFrame();

    virtual void preload_root_assets_pack();

    bool started_ = false;


    // after root pack resources are loaded
    virtual void onAppStart() {}

    virtual void onUpdateFrame(float dt) { (void) dt; }

    virtual void onPreRender() {}

    virtual void onRenderSceneBefore() {}

    virtual void onRenderSceneAfter() {}

    virtual void onFrameEnd() {}
};

void launcher_on_frame();

void setup_resource_managers();

template<typename T>
inline void run_app() {
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

#ifdef EK_DEV_TOOLS
    init_editor_config();
    if (g_editor_config->width > 0 && g_editor_config->height > 0) {
        ek_app.config.window_width = g_editor_config->width;
        ek_app.config.window_height = g_editor_config->height;
    }
#endif

    ek_app.on_ready = [] { init_game_app(new T()); };
    ek_app.on_frame = launcher_on_frame;
    ek_app.on_event = root_app_on_event;
}

}

#endif // SCENEX_BASIC_APPLICATION_H
