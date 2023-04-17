#ifndef SCENEX_ROOT_APP_LISTENER_H
#define SCENEX_ROOT_APP_LISTENER_H

#include <ek/app.h>

#ifdef __cplusplus
extern "C" {
#endif

void root_app_on_frame(void);

void root_app_on_event(ek_app_event ev);

// call virtual method if `g_game_app` exists
void terminate_game_app(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_ROOT_APP_LISTENER_H
