#ifndef GOODIES_SIMPLE_ANIMATOR_H
#define GOODIES_SIMPLE_ANIMATOR_H

#include <ek/math.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct simple_animator {
    float rotation;
    float rotation_speed;
    float hue;
    float hue_speed;
    float hue_mixup_factor;
    color_t base_color;
} simple_animator_t;

void simple_animator_init(void);

void simple_animator_update(float dt);

simple_animator_t* simple_animator_add(entity_t e);

#ifdef __cplusplus
}
#endif

#endif