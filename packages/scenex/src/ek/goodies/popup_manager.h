#ifndef SCENEX_GOODIES_POPUP_MANAGER_H
#define SCENEX_GOODIES_POPUP_MANAGER_H

#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    POPUPS_MAX_COUNT = 8,
};

struct PopupManager {
    entity_t entity;
    entity_t back;
    entity_t layer;
    entity_t closing_last;

    float fade_progress;
    float fade_duration;
    float fade_alpha;

    uint32_t active_num;
    entity_t active[POPUPS_MAX_COUNT];
};

void init_basic_popup(entity_t e);

void open_popup(entity_t e);

void close_popup(entity_t e);

void clear_popups();

void close_all_popups();

void update_popup_managers(float dt);

uint32_t count_active_popups();

extern PopupManager g_popup_manager;

void popup_manager_init();
void update_popup_manager();

#ifdef __cplusplus
};
#endif

#endif // SCENEX_GOODIES_POPUP_MANAGER_H
