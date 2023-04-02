#include "popup_manager.h"
#include <ek/canvas.h>

#include <ek/scenex/base/destroy_timer.h>
#include <ek/scenex/scene_factory.h>
#include <ek/scenex/base/interactiv.h>
#include <ek/scenex/interaction_system.h>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>

#include <ek/scenex/base/tween.h>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/NodeEvents.hpp>

const float tweenDelay = 0.05f;
const float tweenDuration = 0.3f;
const float animationVertDistance = 200.0f;

static entity_t* find_entity(entity_t* entities, uint32_t n, entity_t entity) {
    for (uint32_t i = 0; i < n; ++i) {
        if (entities[i].id == entity.id) {
            return entities + i;
        }
    }
    return NULL;
}

void on_popup_pause(entity_t e) {
    set_touchable(e, false);
}

void on_popup_resume(entity_t e) {
    set_touchable(e, true);
}

void on_popup_opening(entity_t e) {
    set_scale(e, vec2(0, 0));
    set_touchable(e, false);
    set_visible(e, true);
}

void on_popup_open_animation(float t, entity_t e) {
    t = saturate(t);
    const float scale = ease_back_out(t);
    const float fly = ease_p3_out(t);
    set_scale_f(e, scale);
    set_y(e, animationVertDistance * (1.0f - fly));
}

void on_popup_opened(entity_t e) {
    set_touchable(e, true);
}

void on_popup_closing(entity_t e) {
    set_touchable(e, false);
}

void on_popup_closed(entity_t e) {
    // erase at
    entity_t* ptr = find_entity(g_popup_manager.active, g_popup_manager.active_num, e);
    if (ptr) {
        --g_popup_manager.active_num;
        for (uint32_t i = ptr - g_popup_manager.active; i < g_popup_manager.active_num; ++i) {
            g_popup_manager.active[i] = g_popup_manager.active[i + 1];
        }
    }

    if (!g_popup_manager.active_num) {
        entity_t pm = g_popup_manager.entity;
        set_touchable(pm, false);
        set_visible(pm, false);
    } else {
        on_popup_resume(entity_t{g_popup_manager.active[g_popup_manager.active_num - 1]});
    }

    set_visible(e, false);
    // TODO: flag auto-delete
    destroy_later(e, 0, 0);
}

void on_popup_close_animation(float t, entity_t e) {
    t = saturate(1 - t);
    const float scale = ease_back_out(t);
    const float fly = ease_p3_out(t);
    set_scale_f(e, scale);
    set_y(e, animationVertDistance * (fly - 1.0f));
}

void init_basic_popup(entity_t e) {
    auto node_close = find(e, H("btn_close"));
    if (node_close.id) {
        interactive_t* i = interactive_get(node_close);
        if (i) {
            i->back_button = true;
            ek::Button* btn = ecs::try_get<ek::Button>(node_close);
            if (btn) {
                ecs::add<ek::NodeEventHandler>(node_close).on(BUTTON_EVENT_CLICK, [](const ek::NodeEventData& event) {
                    close_popup(get_parent(event.receiver));
                });
            }
        }
    }
}

void open_popup(entity_t e) {
    g_popup_manager.closing_last = NULL_ENTITY;

    // if we have entity in active list - do nothing
    entity_t* found = find_entity(g_popup_manager.active, g_popup_manager.active_num, e);
    if (found) {
        return;
    }

    if (ecs::get<node_t>(e).parent.id == g_popup_manager.layer.id) {
        return;
    }

    if (g_popup_manager.active_num) {
        on_popup_pause(g_popup_manager.active[g_popup_manager.active_num - 1]);
    }
    g_popup_manager.active[g_popup_manager.active_num++] = e;
    on_popup_opening(e);

    tween_t* tween = tween_reset(e);
    tween->delay = tweenDelay;
    tween->duration = tweenDuration;
    tween->advanced = [](entity_t e, float r) {
        if (r >= 1.0f) {
            on_popup_opened(e);
        }
        on_popup_open_animation(r, e);
    };

    append(g_popup_manager.layer, e);
    set_touchable(g_popup_manager.entity, true);
    set_visible(g_popup_manager.entity, true);
}


void close_popup(entity_t e) {
    // we cannot close entity if it is not active
    entity_t* found = find_entity(g_popup_manager.active, g_popup_manager.active_num, e);
    if (!found) {
        return;
    }

    if (g_popup_manager.active[g_popup_manager.active_num - 1].id == e.id) {
        g_popup_manager.closing_last = e;
    }
    on_popup_closing(e);

    tween_t* tween = tween_reset(e);
    tween->delay = tweenDelay;
    tween->duration = tweenDuration;
    tween->advanced = [](entity_t e, float t) {
        on_popup_close_animation(t, e);
        if (t >= 1.0f) {
            on_popup_closed(e);
        }
    };
}

uint32_t count_active_popups() {
    return g_popup_manager.active_num;
}

void clear_popups() {

    g_popup_manager.closing_last = NULL_ENTITY;
    g_popup_manager.fade_progress = 0.0f;
    set_alpha(g_popup_manager.back, 0);

    destroy_children(g_popup_manager.layer);
    g_popup_manager.active_num = 0;

    const entity_t e = g_popup_manager.entity;
    set_touchable(e, false);
    set_visible(e, false);
}

void close_all_popups() {
    entity_t copy[POPUPS_MAX_COUNT];
    uint32_t n = g_popup_manager.active_num;
    memcpy(copy, g_popup_manager.active, sizeof copy);
    for (uint32_t i = 0; i < n; ++i) {
        close_popup(copy[i]);
    }
}

entity_t createBackQuad() {
    entity_t e = create_node2d(H("back"));
    auto& display = ecs::add<ek::Display2D>(e);
    ek::quad2d_setup(e)->setColor(COLOR_BLACK);
    display.program = R_SHADER_SOLID_COLOR;
    ecs::add<ek::LayoutRect>(e)
            .fill(true, true)
            .setInsetsMode(false);

    // intercept back-button if popup manager is active
    interactive_add(e);
    auto& eh = ecs::add<ek::NodeEventHandler>(e);
    eh.on(INTERACTIVE_EVENT_BACK_BUTTON, [](const ek::NodeEventData& ev) {
        ev.processed = true;
    });

    // if touch outside of popups, simulate back-button behavior
    eh.on(POINTER_EVENT_DOWN, [](auto) {
        if (g_popup_manager.active_num) {
            interaction_system_send_back_button();
        }
    });

    return e;
}

PopupManager g_popup_manager;

void popup_manager_init() {
    using namespace ek;

    g_popup_manager.fade_progress = 0.0f;
    g_popup_manager.fade_duration = 0.3f;
    g_popup_manager.fade_alpha = 0.5f;

    auto e = create_node2d(H("popups"));
    g_popup_manager.entity = e;
    auto& pm = g_popup_manager;
    pm.back = createBackQuad();
    append(e, pm.back);

    pm.layer = create_node2d(H("layer"));
    ecs::add<LayoutRect>(pm.layer)
            .enableAlignX(0.5f)
            .enableAlignY(0.5f)
            .setInsetsMode(false);
    append(e, pm.layer);

    // initially popup manager is deactivated
    set_touchable(e, false);
    set_visible(e, false);
}

void update_popup_manager() {
    using namespace ek;
    auto& p = g_popup_manager;
    if (UNLIKELY(!p.entity.id)) {
        return;
    }
    auto dt = g_time_layers[TIME_LAYER_UI].dt;
    bool need_fade = p.active_num != 0;
    if (p.active_num == 1 && p.active[0].id == p.closing_last.id) {
        need_fade = false;
    }
    p.fade_progress = reach(p.fade_progress,
                            need_fade ? 1.0f : 0.0f,
                            dt / p.fade_duration);

    set_alpha_f(p.back, p.fade_alpha * p.fade_progress);

//    if (!p.active.empty()) {
//        auto front = p.active.back();
//        if (ecs::has<close_timeout>(front)) {
//            auto& t = ecs::get<close_timeout>(front);
//            t.time -= dt;
//            if (t.time <= 0.0f) {
//                ecs::remove<close_timeout>(front);
//                close_popup(front);
//            }
//        }
//    }
}

