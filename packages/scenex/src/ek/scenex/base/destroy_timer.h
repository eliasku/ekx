#ifndef SCENEX_DESTROY_TIMER_H
#define SCENEX_DESTROY_TIMER_H

#include <ecx/ecx.h>
#include <sce/util/time_layers.h>
#include <ek/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    entity_t entity;
    float delay;
    time_layer_t time_layer;
} destroy_timer_t;

typedef struct {
    destroy_timer_t* timers;
} destroy_manager_t;

void destroy_later(entity_t e, float delay, time_layer_t timer);

void destroy_children_later(entity_t e, float delay, time_layer_t timer);

extern destroy_manager_t g_destroy_manager;

void update_destroy_queue(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_DESTROY_TIMER_H
