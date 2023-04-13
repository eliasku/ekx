#ifndef SCENEX_GOODIES_FIREWORKS_H
#define SCENEX_GOODIES_FIREWORKS_H

#include <ecx/ecx.h>
#include <ekx/app/time_layers.h>

//#ifdef __cplusplus
//extern "C" {
//#endif
//

struct fireworks_state_t {
    entity_t layer = NULL_ENTITY;
    TimeLayer time_layer = TIME_LAYER_ROOT;
    float timer_ = 1.0f;
    bool enabled = false;
};

void start_fireworks(entity_t e);

void update_fireworks();

void stop_fireworks();

#endif // SCENEX_GOODIES_FIREWORKS_H
