#pragma once

#include <ek/app.h>
#include <ek/gfx.h>
#include <sokol/util/sokol_gfx_imgui.h>

// dearImGUI backend for ek-app + sokol-imgui
#ifdef __cplusplus
extern "C" {
#endif

void ekimgui_setup(void);
void ekimgui_shutdown(void);
void ekimgui_on_event(ek_app_event event);
void ekimgui_on_frame_completed();
void ekimgui_begin_frame(float dt);
void ekimgui_end_frame(void);

extern sg_imgui_t sokol_gfx_gui_state;

#ifdef __cplusplus
}
#endif

