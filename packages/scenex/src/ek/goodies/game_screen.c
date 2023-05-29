#include "game_screen.h"

#include <ek/log.h>
#include <ek/buf.h>
#include <ek/scenex/base/node_events.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/layout_rect.h>
#include <sce/util/time_layers.h>

struct game_screen_manager_ game_screen_manager;

void init_game_screen(entity_t e, string_hash_t name) {
    if (name) {
        set_tag(e, name);
    }
    set_visible(e, false);
    set_touchable(e, false);
}

static void emit_screen_event(entity_t e, string_hash_t event_type) {
    const node_event_t event = node_event(event_type, e);
    emit_node_event(e, &event);
}

static game_screen_trans_t game_screen_trans(void) {
    game_screen_trans_t r = INIT_ZERO;
    r.prevTimeEnd = 0.5;
    r.nextTimeStart = 0.5;
    r.nextTimeEnd = 1;
    r.duration = 0.65;
    r.delay = 0.15;
    return r;
}

static void beginPrev(game_screen_trans_t* state) {
    if (state->prev.id) {
        emit_screen_event(state->prev, GAME_SCREEN_EVENT_EXIT_BEGIN);
    }
    state->prevPlayStarted = true;
}

static void completePrev(game_screen_trans_t* state) {
    if (state->prev.id) {
        set_visible(state->prev, false);
        set_touchable(state->prev, false);
        emit_screen_event(state->prev, GAME_SCREEN_EVENT_EXIT);
    }
    state->prev = NULL_ENTITY;
    state->prevPlayCompleted = true;
}

static void beginNext(game_screen_trans_t* state) {
    state->nextPlayStarted = true;
    if (state->next.id) {
        set_visible(state->next, true);
        set_touchable(state->next, true);
        emit_screen_event(state->next, GAME_SCREEN_EVENT_ENTER_BEGIN);
    }
}

static void completeNext(game_screen_trans_t* state) {
    if (state->next.id) {
        emit_screen_event(state->next, GAME_SCREEN_EVENT_ENTER);
    }
    state->next = NULL_ENTITY;
    state->nextPlayCompleted = true;
}

float game_screen_trans_prev_progress(const game_screen_trans_t* state) {
    return saturate((state->t - state->prevTimeStart) / (state->prevTimeEnd - state->prevTimeStart));
}

float game_screen_trans_next_progress(const game_screen_trans_t* state) {
    return saturate((state->t - state->nextTimeStart) / (state->nextTimeEnd - state->nextTimeStart));
}

static void checkStates(game_screen_trans_t* state) {
    if (!state->prevPlayStarted && state->t >= state->prevTimeStart) {
        state->prevPlayStarted = true;
        beginPrev(state);
    }
    if (!state->prevPlayCompleted && state->t >= state->prevTimeEnd) {
        state->prevPlayCompleted = true;
        completePrev(state);
    }
    if (!state->nextPlayStarted && state->t >= state->nextTimeStart) {
        state->nextPlayStarted = true;
        beginNext(state);
    }
    if (!state->nextPlayCompleted && state->t >= state->nextTimeEnd) {
        state->nextPlayCompleted = true;
        completeNext(state);
    }
}

/** GameScreenManager **/

static void applyTransitionEffect(void) {
    if (game_screen_manager.effect) {
        game_screen_manager.effect();
    }
}

void game_screen_set(string_hash_t name) {
    if (game_screen_manager.transition.active) {
        return;
    }

    // hide all activated screens
    arr_for(it, game_screen_manager.stack) {
        set_visible(*it, false);
        set_touchable(*it, false);
    }

    arr_clear(game_screen_manager.stack);

    entity_t e = game_screen_find(name);
    if (e.id) {
        arr_push(game_screen_manager.stack, e);

        set_visible(e, true);
        set_touchable(e, true);

        // bring to the top
        remove_from_parent(e);
        append(game_screen_manager.layer, e);

        // set fully shown transform
        game_screen_manager.transition = game_screen_trans();
        game_screen_manager.transition.next = e;
        game_screen_manager.transition.t = 1.0f;
        applyTransitionEffect();

        emit_screen_event(e, GAME_SCREEN_EVENT_ENTER_BEGIN);

        // TODO:
        //analytics::screen(name.c_str());
    }
}

entity_t game_screen_find(string_hash_t name) {
    entity_t e = find(game_screen_manager.layer, name);
    if (!e.id) {
        log_debug("could not find screen %s (%08X)", hsp_get(name), name);
    }
    return e;
}

void game_screen_change(string_hash_t name) {
    if (game_screen_manager.transition.active) {
        return;
    }
    game_screen_manager.transition = game_screen_trans();
    game_screen_manager.transition.active = true;
    game_screen_manager.transition.delayTimer = game_screen_manager.transition.delay;
    game_screen_manager.transition.screenRect = find_parent_layout_rect(game_screen_manager.layer, false);
    set_touchable(game_screen_manager.layer, false);

    if (arr_size(game_screen_manager.stack)) {
        game_screen_manager.transition.prev = *arr_back(game_screen_manager.stack);
        arr_pop(game_screen_manager.stack);
    } else {
        game_screen_manager.transition.t = game_screen_manager.transition.nextTimeStart;
        game_screen_manager.transition.delayTimer = 0.0f;
    }

    game_screen_manager.transition.next = game_screen_find(name);
    if (game_screen_manager.transition.next.id) {
        arr_push(game_screen_manager.stack, game_screen_manager.transition.next);

        // bring to the top
        remove_from_parent(game_screen_manager.transition.next);
        append(game_screen_manager.layer, game_screen_manager.transition.next);
    }

    checkStates(&game_screen_manager.transition);
    applyTransitionEffect();

    // TODO:
    //analytics::screen(name.c_str());
}

void update_game_screens(void) {
    if (!game_screen_manager.transition.active) {
        return;
    }
    const float dt = g_time_layers[TIME_LAYER_UI].dt;
    if (game_screen_manager.transition.delayTimer > 0.0f) {
        game_screen_manager.transition.delayTimer -= dt;
    } else if (game_screen_manager.transition.t <= 1.0f) {
        game_screen_manager.transition.t += dt * (1.0f / game_screen_manager.transition.duration);
        if (game_screen_manager.transition.t > 1.0f) {
            game_screen_manager.transition.t = 1.0f;
        }

        applyTransitionEffect();
        checkStates(&game_screen_manager.transition);

        if (game_screen_manager.transition.nextPlayCompleted && game_screen_manager.transition.prevPlayCompleted) {
            game_screen_manager.transition.active = false;
            set_touchable(game_screen_manager.layer, true);
        }
    }
}

static void defaultTransitionEffect(void) {
    game_screen_trans_t* state = &game_screen_manager.transition;
    const entity_t next = state->next;
    const entity_t prev = state->prev;

    if (prev.id) {
        const float t = game_screen_trans_prev_progress(state);
        transform2d_t* transform = get_transform2d(prev);
        const float r = ease_p2_in(t);

        transform->color.scale.a = unorm8_f32_clamped(1.0f - r);
        //transform.color.setAdditive(r * r);
        const float s = 1.0f + r * 0.3f;
        transform2d_set_scale_f(transform, s);
        transform2d_set_position_with_pivot_origin(transform, vec2(0,0), vec2(0, 0), rect_center(state->screenRect));
    }

    if (next.id) {
        const float t = game_screen_trans_next_progress(state);
        transform2d_t* transform = get_transform2d(next);
        const float r = ease_p2_out(t);
        transform->color.scale.a = unorm8_f32_clamped(r);
        //transform.color.offset.a = unorm8_f32_clamped(((1.0f - r) * (1.0f - r)));
        const float s = 1.0f + (1.0f - r) * 0.3f;
        transform2d_set_scale_f(transform, s);
        transform2d_set_position_with_pivot_origin(transform, vec2(0,0), vec2(0, 0), rect_center(state->screenRect));
    }
}

void init_game_screen_manager(void) {
    EK_ASSERT(!game_screen_manager.layer.id);
    game_screen_manager.effect = defaultTransitionEffect;
    game_screen_manager.transition = game_screen_trans();
}
