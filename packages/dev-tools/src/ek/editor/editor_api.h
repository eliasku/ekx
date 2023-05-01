#ifndef EDITOR_API_H
#define EDITOR_API_H

#include <ek/app.h>

#ifdef __cplusplus
extern "C" {
#endif

// load editor config and set window size from prev session
void editor_load(void);

// create editor instance
void editor_setup(void);

// life-cycle

void editor_render_overlay(void);

void editor_update(void);

void editor_before_frame_begin(void);

void editor_pre_render(void);

void editor_post_frame(void);

void editor_event(ek_app_event event);

#ifdef __cplusplus
}
#endif

#endif // EDITOR_API_H
