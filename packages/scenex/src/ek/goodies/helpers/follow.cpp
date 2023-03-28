#include "follow.h"

#include <ecx/ecx.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/node.h>
#include <ek/scenex/interaction_system.h>

void update_target_follow_comps(float dt) {
    for (auto e: ecs::view<target_follow_comp>()) {
        auto& data = ecs::get<target_follow_comp>(e);
        ++data.counter;
        data.time_accum += dt;
        if (data.counter >= data.n) {

            auto& tr = ecs::get<transform2d_t>(e);

            if (is_entity(data.target_entity)) {
                auto parent = get_parent(e);
                if (parent.id) {
                    data.target = local_to_local(data.target_entity, parent, {});
                }
            } else {
                data.target_entity = NULL_ENTITY;
            }

            auto current = vec2_t(tr.getPosition()) - data.offset;
            if (data.integration == FOLLOW_INTEGRATION_EXP) {
                const float c = logf(1.0f - data.k) * data.fixed_frame_rate;
                current = current + (data.target - current) * (1.0f - exp(c * data.time_accum));
            } else if (data.integration == FOLLOW_INTEGRATION_STEPS) {
                const float timeStep = 1.0f / data.fixed_frame_rate;
                while (data.time_accum >= timeStep) {
                    current = current + (data.target - current) * data.k;
                    data.time_accum -= timeStep;
                }
                current = current + (data.target - current) * data.k * (data.time_accum * data.fixed_frame_rate);
            } else if (data.integration == FOLLOW_INTEGRATION_NONE) {
                current = current + (data.target - current) * data.k * (data.time_accum * data.fixed_frame_rate);
            }

            tr.set_position(data.offset + current);
            data.counter = 0;
            data.time_accum = 0.0f;
        }
    }
}

void update_mouse_follow_comps(void) {
    using namespace ek;
    for (auto e: ecs::view<mouse_follow_comp>()) {
        const entity_t parent = get_parent(e);
        if (parent.id) {
            const Camera2D& camera = ecs::get<Camera2D>(Camera2D::Main);
            const vec2_t camera_pointer = vec2_transform(
                    g_interaction_system.pointerScreenPosition_,
                    camera.screenToWorldMatrix
            );
            const vec2_t pos = global_to_local(parent, camera_pointer);
            set_position(e, pos);
        }
    }
}
