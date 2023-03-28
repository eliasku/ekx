#include "interaction_system.h"

#include <ecx/ecx.hpp>
#include <ek/scenex/base/interactiv.h>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/systems/hitTest.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/RenderSystem2D.hpp>
#include <ek/scenex/base/node.h>

struct interaction_system g_interaction_system;

void init_interaction_system() {
    // zero by default
    //g_interaction_system = {};
}

// prev - 0
// curr - 1
entity_t* get_targets(uint32_t off) {
    return g_interaction_system.targetLists[(g_interaction_system.targetListIndex + off) & 1];
}

bool dispatch_interactive_event(entity_t e, const ek::NodeEventData& data) {
    if (is_touchable(e)) {
        auto* eh = ecs::try_get<ek::NodeEventHandler>(e);
        if (eh) {
            data.receiver = e;
            eh->emit(data);
            if (data.processed) {
                return true;
            }
        }
        auto it = get_last_child(e);
        while (it.id) {
            auto prev = get_prev_child(it);
            if (dispatch_interactive_event(it, data)) {
                return true;
            }
            it = prev;
        }
    }
    return false;
}

bool dispatch_back_event(entity_t e, const ek::NodeEventData& data) {
    if (is_touchable(e)) {
        bool processed = false;
        auto* eh = ecs::try_get<ek::NodeEventHandler>(e);
        if (eh) {
            data.receiver = e;
            eh->emit(data);
            if (data.processed) {
                processed = true;
            }
        }
        auto* i = interactive_get(e);
        if (i && i->back_button) {
            i->ev_tap = true;
            i->ev_tap_back = true;
            processed = true;
        }
        if (processed) {
            return true;
        }

        entity_t it = get_last_child(e);
        while (it.id) {
            auto prev = get_prev_child(it);
            if (dispatch_back_event(it, data)) {
                return true;
            }
            it = prev;
        }
    }
    return false;
}

bool interaction_system_list_contains_target(const entity_t list[32], entity_t e) {
    for (uint32_t i = 0; i < 32; ++i) {
        entity_t target = list[i];
        if (target.id == e.id) {
            return true;
        }
        if (!target.id) {
            break;
        }
    }
    return false;
}

//prev = true, bool onlyIfChanged = false
void fire_interaction(string_hash_t event, bool prev, bool onlyIfChanged) {
    uint32_t off = prev ? 0 : 1;
    auto* targets = get_targets(off);
    auto* oppositeTargets = get_targets(off + 1);

    for (uint32_t i = 0; i < 32; ++i) {
        entity_t target = targets[i];
        if (is_entity(target)) {
            // TODO: we actually could check NodeEventHandler and dispatch events, if Interactive component set - we
            //  just update state
            auto* interactive = interactive_get(target);
            if (interactive && !(onlyIfChanged && interaction_system_list_contains_target(oppositeTargets, target))) {
                interactive_handle(interactive, target, event);
            }
        } else {
            break;
        }
    }
}

entity_t global_hit_test(vec2_t* worldSpacePointer, entity_t* capturedCamera) {
    auto& cameras = ek::Camera2D::getCameraQueue();
    for (int i = static_cast<int>(cameras.size()) - 1; i >= 0; --i) {
        auto e = cameras[i];
        if (is_entity(e)) {
            auto* camera = ecs::try_get<ek::Camera2D>(e);
            if (camera && camera->enabled && camera->interactive &&
                rect_contains(camera->screenRect, g_interaction_system.pointerScreenPosition_)) {
                const auto pointerWorldPosition = vec2_transform(g_interaction_system.pointerScreenPosition_,
                                                                 camera->screenToWorldMatrix);
                auto target = ek::hitTest2D(camera->root, pointerWorldPosition);
                if (target.id) {
                    *worldSpacePointer = pointerWorldPosition;
                    *capturedCamera = e;
                    return target;
                }
            }
        }
    }
    return NULL_ENTITY;
}

ek_mouse_cursor search_interactive_targets(entity_t list[32]) {
    vec2_t pointer = {};
    entity_t it;
    entity_t camera;
    entity_t drag_entity = g_interaction_system.dragEntity_;
    if (is_entity(drag_entity)) {
        it = drag_entity;
        interactive_t* interactive = interactive_get(it);
        if (interactive && is_entity(interactive->camera)) {
            camera = interactive->camera;
            pointer = vec2_transform(g_interaction_system.pointerScreenPosition_,
                                     ecs::get<ek::Camera2D>(camera).screenToWorldMatrix);
        }
    } else {
        it = global_hit_test(&pointer, &camera);
    }
    g_interaction_system.hitTarget_ = it;

    auto cursor = EK_MOUSE_CURSOR_PARENT;
    uint32_t len = 0;
    while (it.id) {
        auto* interactive = interactive_get(it);
        if (interactive) {
            interactive->pointer = pointer;
            interactive->camera = camera;
            if (cursor == EK_MOUSE_CURSOR_PARENT) {
                cursor = interactive->cursor;
            }
            EK_ASSERT(len < 32);
            list[len++] = it;
            if (!interactive->bubble) {
                break;
            }
        }
        it = get_parent(it);
    }
    EK_ASSERT(len < 32);
    list[len] = NULL_ENTITY;
    return cursor;
}

void update_interaction_system() {
    using namespace ek;

    entity_t* currTargets = get_targets(1);
    // clear current list
    currTargets[0] = NULL_ENTITY;

    //pointer_global_space = float2::zero;
    auto cursor = EK_MOUSE_CURSOR_PARENT;
    bool changed = false;
    if (g_interaction_system.mouseActive_) {
        g_interaction_system.pointerScreenPosition_ = g_interaction_system.mousePosition0_;
        changed = true;
    } else if (g_interaction_system.touchID_ != 0) {
        g_interaction_system.pointerScreenPosition_ = g_interaction_system.touchPosition0_;
        changed = true;
    }

    if (changed) {
        cursor = search_interactive_targets(currTargets);
    }

    fire_interaction(POINTER_EVENT_OVER, false, true);
    fire_interaction(POINTER_EVENT_OUT, true, true);

    // swap target lists
    g_interaction_system.targetListIndex = (++g_interaction_system.targetListIndex) & 1;

    ek_app_set_mouse_cursor(cursor);
}

void interaction_system_handle_touch(const ek_app_event* event, vec2_t position) {
    const ek_app_event_type type = event->type;
    const uint64_t touch_id = event->touch.id;
    if (type == EK_APP_EVENT_TOUCH_START) {
        if (g_interaction_system.touchID_ == 0) {
            g_interaction_system.touchID_ = touch_id;
            g_interaction_system.touchPosition0_ = position;
            g_interaction_system.mouseActive_ = false;
            g_interaction_system.pointerDown_ = true;
            update_interaction_system();
            fire_interaction(POINTER_EVENT_DOWN, true, false);
        }
    }

    if (g_interaction_system.touchID_ == touch_id) {
        if (type == EK_APP_EVENT_TOUCH_END) {
            g_interaction_system.touchID_ = 0;
            g_interaction_system.touchPosition0_ = {};
            g_interaction_system.pointerDown_ = false;
            fire_interaction(POINTER_EVENT_UP, true, false);
        } else {
            g_interaction_system.touchPosition0_ = position;
        }
    }
}

void interaction_system_handle_mouse(const ek_app_event* event, vec2_t position) {
    const ek_app_event_type type = event->type;
    if (type == EK_APP_EVENT_MOUSE_DOWN) {
        g_interaction_system.mousePosition0_ = position;
        g_interaction_system.pointerDown_ = true;
        fire_interaction(POINTER_EVENT_DOWN, true, false);
    } else if (type == EK_APP_EVENT_MOUSE_UP) {
        g_interaction_system.mousePosition0_ = position;
        g_interaction_system.pointerDown_ = false;
        fire_interaction(POINTER_EVENT_UP, true, false);
    } else if (type == EK_APP_EVENT_MOUSE_MOVE) {
        g_interaction_system.mousePosition0_ = position;
        g_interaction_system.mouseActive_ = true;
        update_interaction_system();
    } else if (type == EK_APP_EVENT_MOUSE_EXIT) {
        g_interaction_system.pointerDown_ = false;
        g_interaction_system.mouseActive_ = false;
        //update();
        fire_interaction(POINTER_EVENT_UP, true, false);
    }
}

void interaction_system_handle_pause(void) {
    ek::broadcast(g_interaction_system.root_, INTERACTIVE_EVENT_SYSTEM_PAUSE);
}

void interaction_system_send_back_button(void) {
    dispatch_back_event(g_interaction_system.root_, {
            INTERACTIVE_EVENT_BACK_BUTTON,
            g_interaction_system.root_, {}
    });
}

void interaction_system_drag(entity_t e) {
    g_interaction_system.dragEntity_ = e;
}

