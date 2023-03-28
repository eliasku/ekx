#ifndef SCENE_INTERACTION_SYSTEM_H
#define SCENE_INTERACTION_SYSTEM_H

#include <ecx/ecx.h>

#include <ek/app.h>
#include <ek/math.h>
#include <ek/hash.h>

#define INTERACTIVE_EVENT_BACK_BUTTON H("back_button")
#define INTERACTIVE_EVENT_SYSTEM_PAUSE H("system_pause")

#ifdef __cplusplus
extern "C" {
#endif

struct interaction_system {
    // screen-space pointer position
    vec2_t pointerScreenPosition_;
    bool pointerDown_;

    entity_t targetLists[2][32];
    int targetListIndex;

    entity_t root_;
    entity_t hitTarget_;

    bool mouseActive_;
    uint64_t touchID_;
    vec2_t touchPosition0_;
    vec2_t mousePosition0_;

    entity_t dragEntity_;
};

extern struct interaction_system g_interaction_system;

void init_interaction_system(void);
void update_interaction_system(void);
void complete_frame_interaction_system(void);
void interaction_system_handle_touch(const ek_app_event* event, vec2_t position);
void interaction_system_handle_mouse(const ek_app_event* event, vec2_t position);
void interaction_system_handle_pause(void);
void interaction_system_drag(entity_t e);
void interaction_system_send_back_button(void);

#ifdef __cplusplus
};
#endif

#endif // SCENE_INTERACTION_SYSTEM_H