#include "transform2d.h"

#include <ek/scenex/base/node.h>

static world_transform2d_t world_transform2d_identity(void) {
    return (world_transform2d_t) {mat3x2_identity(), color2_identity()};
}

static transform2d_t transform2d_identity(void) {
    return (transform2d_t) {{mat3x2_identity()}, color2_identity(), vec2(1, 1), vec2(0, 0)};
}


ECX_DEFINE_TYPE(transform2d_t);
#define Transform2D ECX_ID(transform2d_t)

static void transform2d_ctor(component_handle_t i) {
    ((transform2d_t*)ECX_ID(transform2d_t).data[0])[i] = transform2d_identity();
    ((world_transform2d_t*)ECX_ID(transform2d_t).data[1])[i] = world_transform2d_identity();
}

void setup_transform2d(void) {
    ECX_TYPE_2(transform2d_t, world_transform2d_t, 512);
    Transform2D.ctor = transform2d_ctor;
}

vec2_t transform_up(entity_t it, entity_t top, vec2_t pos) {
    vec2_t result = pos;
    while (it.id && it.id != top.id) {
        const transform2d_t* transform = get_transform2d(it);
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
        const transform2d_t* transform = get_transform2d(it);
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
        transform2d_t* transform = get_transform2d(it);
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
        transform2d_t* transform = get_transform2d(it);
        if (transform) {
            vec2_transform_inverse(pos, transform->matrix, &pos);
        }
        it = get_parent(it);
    }
    return pos;
}

/** transformations after invalidation (already have world matrix) **/
void fast_local_to_local(entity_t src, entity_t dst, vec2_t pos, vec2_t* out) {
    pos = vec2_transform(pos, get_world_transform2d(src)->matrix);
    vec2_transform_inverse(pos, get_world_transform2d(dst)->matrix, out);
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
    transform2d_t* l_data = (transform2d_t*) Transform2D.data[0];
    world_transform2d_t* w_data = (world_transform2d_t*) Transform2D.data[1];
    node_t* node_data = (node_t*) ECX_ID(node_t).data[0];
    component_handle_t handle = get_component_handle(&Transform2D, root);
    /// copy transforms for all roots
    w_data[handle].matrix = l_data[handle].matrix;
    w_data[handle].color = l_data[handle].color;
    ///

    while (begin < end) {
        for (uint32_t i = begin; i < end; ++i) {
            const entity_t parent = out[i];
            const component_handle_t parent_transform_handle = get_component_handle(&Transform2D, parent);
            const world_transform2d_t tp = w_data[parent_transform_handle];

            entity_t it = node_data[get_component_handle(&ECX_ID(node_t), parent)].child_first;
            while (it.id) {
                const component_handle_t it_handle = get_component_handle(&Transform2D, it);
                world_transform2d_t* tw = &w_data[it_handle];
                const transform2d_t tl = l_data[it_handle];
                tw->matrix = mat3x2_mul(tp.matrix, tl.matrix);
                color2_mul(&tw->color, tp.color, tl.color);

                EK_ASSERT(out_size + 1 < TRANSFORM_UPDATE_QUEUE_MAX_SIZE);
                out[out_size++] = it;

                it = node_data[get_component_handle(&ECX_ID(node_t), it)].sibling_next;
            }
        }
        begin = end;
        end = out_size;
    }
}

void transform2d_set_matrix(transform2d_t* t, const mat3x2_t m) {
    t->matrix = m;
    t->cached_scale = mat2_get_scale(m.rot);
    t->cached_skew = mat2_get_skew(m.rot);
}

void transform2d_set_position_with_pivot(transform2d_t* t, vec2_t position, vec2_t pivot) {
    const float xx = -pivot.x;
    const float yy = -pivot.y;
    t->x = position.x + t->matrix.a * xx + t->matrix.c * yy;
    t->y = position.y + t->matrix.d * yy + t->matrix.b * xx;
}

void transform2d_set_position_with_pivot_origin(transform2d_t* t, vec2_t position, vec2_t pivot, vec2_t origin) {
    const float x = position.x + origin.x;
    const float y = position.y + origin.y;
    const float xx = -origin.x - pivot.x;
    const float yy = -origin.y - pivot.y;
   t->x = x + t->matrix.a * xx + t->matrix.c * yy;
   t->y = y + t->matrix.d * yy + t->matrix.b * xx;
}

void transform2d_update_matrix(transform2d_t* t) {
    t->matrix.a = cosf(t->cached_skew.y) * t->cached_scale.x;
    t->matrix.b = sinf(t->cached_skew.y) * t->cached_scale.x;
    t->matrix.c = -sinf(t->cached_skew.x) * t->cached_scale.y;
    t->matrix.d = cosf(t->cached_skew.x) * t->cached_scale.y;
}

void transform2d_scale(transform2d_t* t, float factor) {
    t->cached_scale.x *= factor;
    t->cached_scale.y *= factor;
    transform2d_update_matrix(t);
}

void transform2d_set_scale_f(transform2d_t* t, float value) {
    t->cached_scale.x = value;
    t->cached_scale.y = value;
    transform2d_update_matrix(t);
}

void transform2d_set_scale(transform2d_t* t, vec2_t value) {
    t->cached_scale = value;
    transform2d_update_matrix(t);
}

void transform2d_set_scale_x(transform2d_t* t, float x) {
    t->cached_scale.x = x;
    transform2d_update_matrix(t);
}

void transform2d_set_scale_y(transform2d_t* t, float y) {
    t->cached_scale.y = y;
    transform2d_update_matrix(t);
}

void transform2d_set_rotation(transform2d_t* t, float value) {
    t->cached_skew.x = value;
    t->cached_skew.y = value;
    const float sn = sinf(value);
    const float cs = cosf(value);
    t->matrix.a = cs * t->cached_scale.x;
    t->matrix.b = sn * t->cached_scale.x;
    t->matrix.c = -sn * t->cached_scale.y;
    t->matrix.d = cs * t->cached_scale.y;
}

float transform2d_get_rotation(const transform2d_t* t) {
//        return skew.x == skew.y ? skew.y : 0.0f;
    return t->cached_skew.y;
}

void transform2d_set_skew(transform2d_t* t, vec2_t value) {
    t->cached_skew = value;
    transform2d_update_matrix(t);
}

void transform2d_set_transform_ex(transform2d_t* t,vec2_t position, vec2_t scale, vec2_t skew, vec2_t pivot) {
    t->cached_scale = scale;
    t->cached_skew = skew;
    transform2d_update_matrix(t);
    transform2d_set_position_with_pivot(t, position, pivot);
}

void transform2d_set_transform(transform2d_t* t,vec2_t position, vec2_t scale, float rotation) {
    t->cached_scale = scale;
    t->cached_skew.x = rotation;
    t->cached_skew.y = rotation;
    const float sn = sinf(rotation);
    const float cs = cosf(rotation);
    t->matrix.a = cs * t->cached_scale.x;
    t->matrix.b = sn * t->cached_scale.x;
    t-> matrix.c = -sn * t->cached_scale.y;
    t->matrix.d = cs * t->cached_scale.y;
    t->matrix.tx = position.x;
    t->matrix.ty = position.y;
}

void transform2d_set_transform_pss(transform2d_t* t,vec2_t position, vec2_t scale, vec2_t skew) {
    t->cached_scale = scale;
    t->cached_skew = skew;
    transform2d_update_matrix(t);
    t->matrix.tx = position.x;
    t->matrix.ty = position.y;
}

void transform2d_rotate(transform2d_t* t,float value) {
    t->cached_skew.x += value;
    t->cached_skew.y += value;
    transform2d_update_matrix(t);
}

void transform2d_translate(transform2d_t* t,vec2_t delta) {
    t->pos = add_vec2(t->pos, delta);
}

void transform2d_lerp_scale(transform2d_t* tr, vec2_t target, float t) {
    transform2d_set_scale(tr, lerp_vec2(tr->cached_scale, target, t));
}

void transform2d_lerp_position(transform2d_t* tr,vec2_t target, float t) {
    tr->pos = lerp_vec2(tr->pos, target, t);
}

void transform2d_lerp_rotation(transform2d_t* tr,float target, float t) {
    transform2d_set_rotation(tr, lerp_f32(tr->cached_skew.y, target, t));
}

// Utilities
vec2_t get_position(entity_t e) {
    return get_transform2d(e)->pos;
}

vec2_t get_scale(entity_t e) {
    return get_transform2d(e)->cached_scale;
}

float get_rotation(entity_t e) {
    return get_transform2d(e)->cached_skew.y;
}

void set_position(entity_t e, vec2_t v) {
    get_transform2d(e)->pos = v;
}

void set_scale(entity_t e, vec2_t v) {
    transform2d_set_scale(get_transform2d(e), v);
}

void set_rotation(entity_t e, float v) {
    transform2d_set_rotation(get_transform2d(e), v);
}

void set_alpha(entity_t e, uint8_t alpha) {
    add_transform2d(e)->color.scale.a = alpha;
}

void set_alpha_f(entity_t e, float alpha) {
    add_transform2d(e)->color.scale.a = unorm8_f32_clamped(alpha);
}

void set_color_additive_f(entity_t e, float additive) {
    add_transform2d(e)->color.offset.a = unorm8_f32_clamped(additive);
}

void set_color(entity_t e, color_t color) {
    add_transform2d(e)->color.scale = color;
}

void set_color_offset(entity_t e, color_t offset) {
    add_transform2d(e)->color.offset = offset;
}

void set_x(entity_t e, float x) {
    add_transform2d(e)->x = x;
}

void set_y(entity_t e, float y) {
    add_transform2d(e)->y = y;
}

void set_scale_f(entity_t e, float xy) {
    set_scale(e, vec2(xy, xy));
}
