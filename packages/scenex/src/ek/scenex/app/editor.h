#ifndef SCENEX_APP_EDITOR_H
#define SCENEX_APP_EDITOR_H

// conditionally provides callbacks to Editor life-cycle

#ifdef EK_DEV_TOOLS

#include <ek/editor/editor_api.h>

#define DRAW_CALLS_MAX 1024

#define EDITOR_LOAD_SETTINGS editor_load()
#define EDITOR_SETUP editor_setup()

#define EDITOR_RENDER_OVERLAY editor_render_overlay()
#define EDITOR_UPDATE editor_update()
#define EDITOR_BEFORE_FRAME_BEGIN editor_before_frame_begin()
#define EDITOR_PRE_RENDER editor_pre_render()
#define EDITOR_POST_FRAME editor_post_frame()
#define EDITOR_EVENT(event) editor_event(event)

#else

#define DRAW_CALLS_MAX 128

#define EDITOR_LOAD_SETTINGS ((void)0)
#define EDITOR_SETUP ((void)0)

#define EDITOR_RENDER_OVERLAY ((void)0)
#define EDITOR_UPDATE ((void)0)
#define EDITOR_BEFORE_FRAME_BEGIN ((void)0)
#define EDITOR_PRE_RENDER ((void)0)
#define EDITOR_POST_FRAME ((void)0)
#define EDITOR_EVENT(event) ((void)event)

#endif // EK_DEV_TOOLS

#endif // SCENEX_APP_EDITOR_H
