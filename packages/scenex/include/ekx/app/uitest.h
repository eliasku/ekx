#ifndef SCENEX_UITEST_H
#define SCENEX_UITEST_H

#ifdef EK_UITEST

#include <ecx/ecx.h>
#include <ek/scenex/app/base_game.h>
#include <ek/scenex/base/node.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int uitest_step;
void uitest_screenshot(const char* name);
void uitest_done();
void uitest_fail();
void uitest_click_entity(entity_t e);
#define uitest_click(...) uitest_click_entity(find_by_path(game_app_state.root, __VA_ARGS__))

void uitest(const char* name, void(*run)(void));
void uitest_setup(void);
void uitest_start(void);

#define UITEST_SETUP uitest_setup()
#define UITEST_START uitest_start()

#ifdef __cplusplus
}
#endif

#else

#define UITEST_SETUP ((void)(0))
#define UITEST_START ((void)(0))

#endif // EK_UITEST

#endif // SCENEX_UITEST_H
