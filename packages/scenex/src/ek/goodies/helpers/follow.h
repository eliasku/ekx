#pragma once

#include <ecx/ecx.hpp>
#include <ek/math.h>

enum follow_integration_mode {
    FOLLOW_INTEGRATION_EXP = 0,
    FOLLOW_INTEGRATION_STEPS = 1,
    FOLLOW_INTEGRATION_NONE = 2,
};

struct target_follow_comp {

    vec2_t offset = {};
    vec2_t target = {};
    entity_t target_entity = NULL_ENTITY;

    float k = 0.1f;
    float fixed_frame_rate = 60.0f;

    int n = 1;
    int counter = 0;
    float time_accum = 0.0f;

    int frame = 0;
    int reset_in = 100;

    follow_integration_mode integration = FOLLOW_INTEGRATION_EXP;
};

struct mouse_follow_comp {};

void update_target_follow_comps(float dt);
void update_mouse_follow_comps(void);
