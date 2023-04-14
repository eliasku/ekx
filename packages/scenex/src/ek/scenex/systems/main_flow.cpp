#include "main_flow.h"

#include <ekx/ng/updater.h>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/particles/particle_system.h>
#include <ek/scenex/interaction_system.h>
#include <ek/scenex/base/destroy_timer.h>
#include <ek/goodies/camera_shaker.h>
#include <ek/goodies/trail2d.h>
#include <ek/goodies/bubble_text.h>
#include <ek/goodies/popup_manager.h>
#include <ek/scenex/2d/camera2d.h>
#include <ek/scenex/2d/layout_rect.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/base/tween.h>
#include <ek/goodies/game_screen.h>

// TODO: ->C
#include <ek/scenex/2d/dynamic_atlas.h>
#include <ek/scenex/2d/Atlas.hpp>

void scene_pre_update(entity_t root, float dt) {
    (void)root;
    update_interaction_system();
    update_time_layers(dt);
    update_audio_manager();
    game_screen_update();

    LayoutRect_update();
    tween_update();
    camera_shaker_update();

    bubble_text_update();
    update_popup_manager();

    Button_update();
    MovieClip_update();

    updater_update();
}

void scene_post_update(entity_t root) {
    update_destroy_queue();
    update_world_transform_2d(root);
    update_trail2d();
    update_emitters();
    update_particles();
    update_camera2d_queue();
    interactive_clear_all_events();
}

void scene_render(entity_t root) {
    (void)root;
    // TODO: ->C
    update_res_dynamic_atlas();
    // TODO: ->C
    update_res_atlas();

    render_camera2d_queue();
}
