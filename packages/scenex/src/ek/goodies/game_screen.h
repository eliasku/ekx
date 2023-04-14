#ifndef SCENEX_GOODIES_GAME_SCREENS_H
#define SCENEX_GOODIES_GAME_SCREENS_H

#include <ecx/ecx.h>
#include <ek/math.h>
#include <ek/hash.h>

#define GAME_SCREEN_EVENT_CREATE H("game-screen-create")
#define GAME_SCREEN_EVENT_DESTROY H("game-screen-destroy")
#define GAME_SCREEN_EVENT_ENTER H("game-screen-enter")
#define GAME_SCREEN_EVENT_ENTER_BEGIN H("game-screen-enter-begin")
#define GAME_SCREEN_EVENT_EXIT H("game-screen-exit")
#define GAME_SCREEN_EVENT_EXIT_BEGIN H("game-screen-exit-begin")

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float t;

    float prevTimeStart;
    float prevTimeEnd;
    float nextTimeStart;
    float nextTimeEnd;

    float duration;
    float delay;
    float delayTimer;

    rect_t screenRect;

    entity_t prev;
    entity_t next;

    bool active;
    bool prevPlayStarted;
    bool prevPlayCompleted;
    bool nextPlayStarted;
    bool nextPlayCompleted;
} game_screen_trans_t;

struct game_screen_manager_ {
    entity_t layer;
    entity_t* stack;
    game_screen_trans_t transition;

    void (* effect)(void);
};

float game_screen_trans_prev_progress(const game_screen_trans_t* state);

float game_screen_trans_next_progress(const game_screen_trans_t* state);

void game_screen_set(string_hash_t name);

entity_t game_screen_find(string_hash_t name);

void game_screen_change(string_hash_t name);

void game_screen_update(void);

void init_game_screen(entity_t e, string_hash_t name);

extern struct game_screen_manager_ game_screen_manager;

void init_game_screen_manager(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_GOODIES_GAME_SCREENS_H
