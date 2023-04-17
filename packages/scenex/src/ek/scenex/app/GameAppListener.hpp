#ifndef SCENEX_GAME_APP_LISTENER_H
#define SCENEX_GAME_APP_LISTENER_H

#include <ek/app.h>

struct GameAppListener {
    virtual ~GameAppListener() = default;

    virtual void onBeforeFrameBegin() {}

    virtual void onPreload() {}

    // used to render all offscreen passes
    virtual void onPreRender() {}

    virtual void onRenderOverlay() {}

    virtual void onRenderFrame() {}

    virtual void onUpdate() {}

    virtual void onStart() {}

    virtual void onEvent(ek_app_event) {}

    virtual void onPostFrame() {}
};

#endif // SCENEX_GAME_APP_LISTENER_H
