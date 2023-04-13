#include "tween.h"
#include "destroy_timer.h"
#include <ek/math.h>

ecx_component_type Tween;

static void Tween_ctor(component_handle_t handle) {
    ((tween_t*) Tween.data[0])[handle] = (tween_t) {0};
}

void Tween_setup(void) {
    const ecx_component_type_decl decl = (ecx_component_type_decl) {
            "Tween",
            8,
            1,
            {sizeof(tween_t)}
    };
    init_component_type(&Tween, decl);
    Tween.ctor = Tween_ctor;
}

void handle_end(entity_t e, tween_t* tween) {
    if (tween->destroy_entity) {
        destroy_later(e, 0, 0);
    } else if (!tween->keep) {
        del_tween(e);
    }
}

void on_tween_completed(entity_t e, tween_t* tween) {
    if (tween->completed) {
        tween->completed(e);
        tween->completed = NULL;
    }
}

void update_frame(entity_t e, tween_t* tween) {
    if (tween->advanced) {
        const float t = saturate(tween->time / tween->duration);
        tween->advanced(e, t);
    }
}

void tween_update(void) {
    for (uint32_t i = Tween.size - 1; i > 0; --i) {
        const entity_t e = get_entity(&Tween, i);
        tween_t* tween = (tween_t*) get_component_data(&Tween, i, 0);
        const float dt = g_time_layers[tween->timer].dt;
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
            tween->advanced = NULL;
            on_tween_completed(e, tween);
            handle_end(e, tween);
        }
    }
}

tween_t* tween_reset(entity_t e) {
    tween_t* tween = add_tween(e);
    if (tween->time > 0.0f && tween->time < tween->duration) {
        tween->time = tween->duration;
        update_frame(e, tween);
        tween->advanced = NULL;
        on_tween_completed(e, tween);
    }
    tween->destroy_entity = false;
    tween->keep = false;
    tween->time = 0.0f;
    return tween;
}
