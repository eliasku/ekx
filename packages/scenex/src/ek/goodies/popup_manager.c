#include "popup_manager.h"

#include <ek/canvas.h>

#include <ek/scenex/base/destroy_timer.h>
#include <ek/scenex/scene_factory.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/interaction_system.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/layout_rect.h>

#include <ek/scenex/base/tween.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/base/node_events.h>
#include <ek/scenex/base/node.h>

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
        for (uint32_t i = (uint32_t)(ptr - g_popup_manager.active); i < g_popup_manager.active_num; ++i) {
            g_popup_manager.active[i] = g_popup_manager.active[i + 1];
        }
    }

    if (!g_popup_manager.active_num) {
        entity_t pm = g_popup_manager.entity;
        set_touchable(pm, false);
        set_visible(pm, false);
    } else {
        on_popup_resume(g_popup_manager.active[g_popup_manager.active_num - 1]);
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

static void on_popup_close_button(const node_event_t* event){
    close_popup(get_parent(event->receiver));
}

void init_basic_popup(entity_t e) {
    entity_t node_close = find(e, H("btn_close"));
    if (node_close.id) {
        interactive_t* i = get_interactive(node_close);
        if (i) {
            i->back_button = true;
            button_t* btn = get_button(node_close);
            if (btn) {
                add_node_event_listener(node_close, BUTTON_EVENT_CLICK, on_popup_close_button);
            }
        }
    }
}

static void popup_open_tween_cb(entity_t e, float t) {
    if (t >= 1.0f) {
        on_popup_opened(e);
    }
    on_popup_open_animation(t, e);
}

static void popup_close_tween_cb(entity_t e, float t) {
    on_popup_close_animation(t, e);
    if (t >= 1.0f) {
        on_popup_closed(e);
    }
}

void open_popup(entity_t e) {
    g_popup_manager.closing_last = NULL_ENTITY;

    // if we have entity in active list - do nothing
    entity_t* found = find_entity(g_popup_manager.active, g_popup_manager.active_num, e);
    if (found) {
        return;
    }

    if (get_node(e)->parent.id == g_popup_manager.layer.id) {
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
    tween->advanced = popup_open_tween_cb;

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
    tween->advanced = popup_close_tween_cb;
}

uint32_t count_active_popups(void) {
    return g_popup_manager.active_num;
}

void clear_popups(void) {
    g_popup_manager.closing_last = NULL_ENTITY;
    g_popup_manager.fade_progress = 0.0f;
    set_alpha(g_popup_manager.back, 0);

    destroy_children(g_popup_manager.layer);
    g_popup_manager.active_num = 0;

    const entity_t e = g_popup_manager.entity;
    set_touchable(e, false);
    set_visible(e, false);
}

void close_all_popups(void) {
    entity_t copy[POPUPS_MAX_COUNT];
    uint32_t n = g_popup_manager.active_num;
    memcpy(copy, g_popup_manager.active, sizeof copy);
    for (uint32_t i = 0; i < n; ++i) {
        close_popup(copy[i]);
    }
}

static void on_popup_back_button(const node_event_t* event) {
    ((node_event_t*)&event)->processed = true;
}

static void on_popup_down_outside(const node_event_t* event) {
    UNUSED(event);
    if (g_popup_manager.active_num) {
        interaction_system_send_back_button();
    }
}

entity_t createBackQuad(void) {
    entity_t e = create_node2d(H("back"));
    set_color_quad(e, rect_01(), COLOR_BLACK);
    get_display2d(e)->program = R_SHADER_SOLID_COLOR;
    LayoutRect_setInsetsMode(
        LayoutRect_fill(add_layout_rect(e), true, true),
            false
    );

    add_interactive(e);

    // intercept back-button if popup manager is active
    add_node_event_listener(e, INTERACTIVE_EVENT_BACK_BUTTON, on_popup_back_button);

    // if touch outside popups, simulate back-button behavior
    add_node_event_listener(e, POINTER_EVENT_DOWN, on_popup_down_outside);

    return e;
}

popup_manager_t g_popup_manager;

void popup_manager_init() {
    g_popup_manager.fade_progress = 0.0f;
    g_popup_manager.fade_duration = 0.3f;
    g_popup_manager.fade_alpha = 0.5f;

    entity_t e = create_node2d(H("popups"));
    g_popup_manager.entity = e;
    g_popup_manager.back = createBackQuad();
    append(e, g_popup_manager.back);

    g_popup_manager.layer = create_node2d(H("layer"));
    layout_rect_t* l = add_layout_rect(g_popup_manager.layer);
    LayoutRect_enableAlignX(l, 0.5f, 0.0f);
    LayoutRect_enableAlignY(l, 0.5f, 0.0f);
    LayoutRect_setInsetsMode(l, false);
    append(e, g_popup_manager.layer);

    // initially popup manager is deactivated
    set_touchable(e, false);
    set_visible(e, false);
}

void update_popup_manager() {
    popup_manager_t* p = &g_popup_manager;
    if (UNLIKELY(!p->entity.id)) {
        return;
    }
    const float dt = g_time_layers[TIME_LAYER_UI].dt;
    bool need_fade = p->active_num != 0;
    if (p->active_num == 1 && p->active[0].id == p->closing_last.id) {
        need_fade = false;
    }
    p->fade_progress = reach(p->fade_progress,
                            need_fade ? 1.0f : 0.0f,
                            dt / p->fade_duration);

    set_alpha_f(p->back, p->fade_alpha * p->fade_progress);

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

