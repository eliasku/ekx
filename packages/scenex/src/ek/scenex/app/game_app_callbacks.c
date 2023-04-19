#include "game_app_callbacks.h"

typedef void(* void_callback_t)(void* userdata);

typedef void(* app_event_callback_t)(void* userdata, ek_app_event event);

game_app_callbacks_t game_app_callbacks;

void game_app_add_callback(game_app_callback_t** list, game_app_callback_t* callback) {
    game_app_callback_t* curr = *list;
    game_app_callback_t* last = NULL;
    while (curr) {
        last = curr;
        curr = curr->next;
    }
    if (last) {
        last->next = callback;
    } else {
        *list = callback;
    }
    callback->next = NULL;
}

static void dispatch_void_list(game_app_callback_t* callback) {
    while (callback) {
        if (callback->fn) {
            ((void_callback_t) callback->fn)(callback->userdata);
        }
        callback = callback->next;
    }
}

void dispatcher_before_frame_begin(void) {
    dispatch_void_list(game_app_callbacks.on_before_frame_begin);
}

void dispatcher_pre_load(void) {
    dispatch_void_list(game_app_callbacks.on_pre_load);
}

void dispatcher_pre_render(void) {
    dispatch_void_list(game_app_callbacks.on_pre_render);
}

void dispatcher_render_overlay(void) {
    dispatch_void_list(game_app_callbacks.on_render_overlay);
}

void dispatcher_render_frame(void) {
    dispatch_void_list(game_app_callbacks.on_render_frame);
}
void dispatcher_post_frame(void) {
    dispatch_void_list(game_app_callbacks.on_post_frame);
}

void dispatcher_update(void) {
    dispatch_void_list(game_app_callbacks.on_update);
}

void dispatcher_start(void) {
    dispatch_void_list(game_app_callbacks.on_start);
}

void dispatcher_event(ek_app_event event) {
    game_app_callback_t* callback = game_app_callbacks.on_event;
    while (callback) {
        if (callback->fn) {
            ((app_event_callback_t) callback->fn)(callback->userdata, event);
        }
        callback = callback->next;
    }
}

