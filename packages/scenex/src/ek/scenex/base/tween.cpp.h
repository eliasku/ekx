#include "tween.h"
#include "destroy_timer.h"
#include <ek/math.h>

void handle_end(entity_t e, tween_t* tween) {
    if (tween->destroy_entity) {
        destroy_later(e, 0, 0);
    } else if (!tween->keep) {
        ecs::remove<tween_t>(e);
    }
}

void on_tween_completed(entity_t e, tween_t* tween) {
    if(tween->completed) {
        tween->completed(e);
        tween->completed = nullptr;
    }
}

void update_frame(entity_t e, tween_t* tween) {
    if (tween->advanced) {
        const float t = saturate(tween->time / tween->duration);
        tween->advanced(e, t);
    }
}

void tween_update(void) {
    for (auto e: ecs::view_backward<tween_t>()) {
        tween_t* tween = (tween_t* )get_component(ecs::type<tween_t>(), e);
        auto dt = g_time_layers[tween->timer].dt;
        if (tween->delay > 0.0f) {
            tween->delay -= dt;
            continue;
        }
        tween->time += dt;
        bool completed = false;
        if (tween->time >= tween->duration) {
            tween->time = tween->duration;
            completed = true;
        }
        update_frame(e, tween);

        if (completed) {
            tween->advanced = nullptr;
            on_tween_completed(e, tween);
            handle_end(e, tween);
        }
    }
}

tween_t* tween_reset(entity_t e) {
    tween_t& tween = ecs::add<tween_t>(e);
    if (tween.time > 0.0f && tween.time < tween.duration) {
        tween.time = tween.duration;
        update_frame(e, &tween);
        tween.advanced = nullptr;
        on_tween_completed(e, &tween);
    }
    tween.destroy_entity = false;
    tween.keep = false;
    tween.time = 0.0f;
    return &tween;
}
