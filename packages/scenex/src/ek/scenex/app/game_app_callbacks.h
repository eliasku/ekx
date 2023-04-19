#ifndef SCENEX_GAME_APP_CALLBACKS_H
#define SCENEX_GAME_APP_CALLBACKS_H

#include <ek/app.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct game_app_callback_ {
    void* userdata;
    void* fn;
    struct game_app_callback_* next;
} game_app_callback_t;

typedef struct {
    game_app_callback_t* on_before_frame_begin;
    game_app_callback_t* on_pre_load;
    game_app_callback_t* on_pre_render;
    game_app_callback_t* on_render_overlay;
    game_app_callback_t* on_render_frame;
    game_app_callback_t* on_post_frame;
    game_app_callback_t* on_update;
    game_app_callback_t* on_start;
    game_app_callback_t* on_event;
} game_app_callbacks_t;

extern game_app_callbacks_t game_app_callbacks;

void game_app_add_callback(game_app_callback_t** list, game_app_callback_t* callback);

void dispatcher_before_frame_begin(void);

void dispatcher_pre_load(void);

void dispatcher_pre_render(void);

void dispatcher_render_overlay(void);

void dispatcher_render_frame(void);

void dispatcher_post_frame(void);

void dispatcher_update(void);

void dispatcher_start(void);

void dispatcher_event(ek_app_event event);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_GAME_APP_CALLBACKS_H
