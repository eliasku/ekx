#include "Transform2D.hpp"

#include <ek/scenex/base/node.h>

vec2_t transform_up(entity_t it, entity_t top, vec2_t pos) {
    vec2_t result = pos;
    while (it.id && it.id != top.id) {
        const transform2d_t* transform = ecs::try_get<transform2d_t>(it);
        if (transform) {
            result = vec2_transform(result, transform->matrix);
        }
        it = get_parent(it);
    }
    return result;
}

vec2_t transform_down(entity_t top, entity_t it, vec2_t pos) {
    vec2_t result = pos;
    while (it.id && it.id != top.id) {
        const transform2d_t* transform = ecs::try_get<transform2d_t>(it);
        if (transform) {
            vec2_transform_inverse(result, transform->matrix, &result);
        }
        it = get_parent(it);
    }
    return result;
}

vec2_t local_to_local(entity_t src, entity_t dst, vec2_t pos) {
    vec2_t result = pos;
    const entity_t lca = find_lower_common_ancestor(src, dst);
    if (lca.id) {
        result = transform_up(src, lca, result);
        result = transform_down(lca, dst, result);
    }
    return result;
}

vec2_t local_to_global(entity_t local, vec2_t local_pos) {
    vec2_t pos = local_pos;
    entity_t it = local;
    while (it.id) {
        transform2d_t* transform = ecs::try_get<transform2d_t>(it);
        if (transform) {
            pos = vec2_transform(pos, transform->matrix);
        }
        it = get_parent(it);
    }
    return pos;
}

vec2_t global_to_local(entity_t local, vec2_t global_pos) {
    vec2_t pos = global_pos;
    entity_t it = local;
    while (it.id) {
        transform2d_t* transform = ecs::try_get<transform2d_t>(it);
        if (transform) {
            vec2_transform_inverse(pos, transform->matrix, &pos);
        }
        it = get_parent(it);
    }
    return pos;
}

/** transformations after invalidation (already have world matrix) **/
void fast_local_to_local(entity_t src, entity_t dst, vec2_t pos, vec2_t* out) {
    pos = vec2_transform(pos, ecs::get<world_transform2d_t>(src).matrix);
    vec2_transform_inverse(pos, ecs::get<world_transform2d_t>(dst).matrix, out);
}

enum {
    TRANSFORM_UPDATE_QUEUE_MAX_SIZE = ECX_ENTITIES_MAX_COUNT - 1,
};

/** Invalidate Transform2D **/
void update_world_transform_2d(entity_t root) {
    entity_t out[TRANSFORM_UPDATE_QUEUE_MAX_SIZE];
    uint32_t out_size = 1;
    out[0] = root;

    uint32_t begin = 0;
    uint32_t end = out_size;
    ecx_component_type* w_type = ecs::type<world_transform2d_t>();
    ecx_component_type* l_type = ecs::type<transform2d_t>();
    ecx_component_type* node_type = ecs::type<node_t>();
    world_transform2d_t* w_data = (world_transform2d_t*) w_type->data[0];
    transform2d_t* l_data = (transform2d_t*) l_type->data[0];
    node_t* node_data = (node_t*) node_type->data[0];
    component_handle_t w_handle = get_component_handle(w_type, root);
    component_handle_t l_handle = get_component_handle(l_type, root);
    /// copy transforms for all roots
    w_data[w_handle].matrix = l_data[l_handle].matrix;
    w_data[w_handle].color = l_data[l_handle].color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const entity_t parent = out[i];
            const world_transform2d_t tp = w_data[get_component_handle(w_type, parent)];

            entity_t it = node_data[get_component_handle(node_type, parent)].child_first;
            while (it.id) {
                world_transform2d_t* tw = &w_data[get_component_handle(w_type, it)];
                const transform2d_t tl = l_data[get_component_handle(l_type, it)];
                tw->matrix = mat3x2_mul(tp.matrix, tl.matrix);
                color2_mul(&tw->color, tp.color, tl.color);

                EK_ASSERT(out_size + 1 < TRANSFORM_UPDATE_QUEUE_MAX_SIZE);
                out[out_size++] = it;

//                auto* node_data = ecs::C<Node>::get_by_entity(it);
//                auto it_next = node_data->sibling_next.index;
//                if(it_next == it) {
//                    log_error("error invalid children in %s", hsp_get(node_data.tag));
//                    it_next = 0;
//                }
//                it = it_next;
                it = node_data[get_component_handle(node_type, it)].sibling_next;
                //it = ecs::C<Node>::get_by_entity(it)->sibling_next.index;
            }
        }
        begin = end;
        end = out_size;
    }
}


// Utilities
vec2_t get_position(entity_t e) {
    return ecs::get<transform2d_t>(e).getPosition();
}

vec2_t get_scale(entity_t e) {
    return ecs::get<transform2d_t>(e).getScale();
}

float get_rotation(entity_t e) {
    return ecs::get<transform2d_t>(e).getRotation();
}

void set_position(entity_t e, vec2_t v) {
    ecs::get<transform2d_t>(e).set_position(v);
}

void set_scale(entity_t e, vec2_t v) {
    ecs::get<transform2d_t>(e).set_scale(v);
}

void set_rotation(entity_t e, float v) {
    ecs::get<transform2d_t>(e).set_rotation(v);
}

void set_alpha(entity_t e, uint8_t alpha) {
    ecs::add<transform2d_t>(e).color.scale.a = alpha;
}

void set_alpha_f(entity_t e, float alpha) {
    ecs::add<transform2d_t>(e).color.scale.a = unorm8_f32_clamped(alpha);
}

void set_color_additive_f(entity_t e, float additive) {
    ecs::add<transform2d_t>(e).color.offset.a = unorm8_f32_clamped(additive);
}

void set_color(entity_t e, color_t color) {
    ecs::add<transform2d_t>(e).color.scale = color;
}

void set_color_offset(entity_t e, color_t offset) {
    ecs::add<transform2d_t>(e).color.offset = offset;
}

void set_x(entity_t e, float x) {
    ecs::add<transform2d_t>(e).setX(x);
}

void set_y(entity_t e, float y) {
    ecs::add<transform2d_t>(e).setY(y);
}

void set_scale_f(entity_t e, float xy) {
    ecs::add<transform2d_t>(e).set_scale(vec2(xy, xy));
}
