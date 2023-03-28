#include "destroy_timer.h"

#include "node.h"
#include <ek/time.h>

destroy_manager_t g_destroy_manager;

void destroy_later(entity_t e, float delay, TimeLayer timer) {
    if(is_entity(e)) {
        destroy_timer_t t;
        t.entity = e;
        t.delay = delay;
        t.time_layer = timer;
        arr_push(g_destroy_manager.timers, t);
    }
}

void destroy_children_later(entity_t e, float delay, TimeLayer timer) {
    entity_t it = get_first_child(e);
    while (it.id) {
        destroy_later(it, delay, timer);
        it = get_next_child(it);
    }
}

void update_destroy_queue() {
    uint32_t i = 0;
    uint32_t end = arr_size(g_destroy_manager.timers);
    while(i < end) {
        destroy_timer_t* timer = &g_destroy_manager.timers[i];
        entity_t e = timer->entity;
        if(is_entity(e)) {
            if (timer->delay > 0.0f) {
                timer->delay -= g_time_layers[timer->time_layer].dt;
                ++i;
                continue;
            }

            if (ecs::has<node_t>(e)) {
                destroy_node(e);
            } else {
                destroy_entity(e);
            }
        }

        --end;
        if(i < end) {
            g_destroy_manager.timers[i] = g_destroy_manager.timers[end];
        }
    }
    if(end < arr_size(g_destroy_manager.timers)) {
        arr_resize(g_destroy_manager.timers, end);
    }
}
