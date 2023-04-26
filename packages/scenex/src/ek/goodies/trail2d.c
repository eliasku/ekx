#include "trail2d.h"
#include <ek/buf.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/canvas.h>
#include <ek/scenex/2d/sprite.h>

ecx_component_type Trail2D;
ecx_component_type TrailRenderer2D;

static void Trail2D_ctor(component_handle_t i) {
    trail2d_t r = INIT_ZERO;
    r.drain_speed = 2;
    r.segment_distance_max = 10;
    r.scale = 1;
    ((trail2d_t*) Trail2D.data[0])[i] = r;
}

static void Trail2D_dtor(component_handle_t i) {
    trail2d_t* r = &((trail2d_t*) Trail2D.data[0])[i];
    arr_reset(r->nodes.data);
}

void setup_trail2d(void) {
    init_component_type(&Trail2D, (ecx_component_type_decl) {"Trail2D", 16, 1, {sizeof(trail2d_t)}});
    init_component_type(&TrailRenderer2D,
                        (ecx_component_type_decl) {"TrailRenderer2D", 16, 1, {sizeof(trail_renderer2d_t)}});
    Trail2D.ctor = Trail2D_ctor;
    Trail2D.dtor = Trail2D_dtor;
}

static void deq_move(trail2d_deque_t* deq) {
    uint32_t dst = 0;
    for (uint32_t i = deq->first; i < deq->end; ++i) {
        deq->data[dst] = deq->data[i];
        ++dst;
    }
    deq->end -= deq->first;
    deq->first = 0;
}

static trail2d_node_t* deq_back(trail2d_deque_t* deq) {
    EK_ASSERT(deq->first < deq->end);
    return deq->data + (deq->end - 1);
}

static void deq_push(trail2d_deque_t* deq, trail2d_node_t el) {
    if (deq->end == deq->capacity && deq->first > 0) {
        deq_move(deq);
    }
    if (deq->end < deq->capacity) {
        deq->data[deq->end++] = el;
    } else {
        arr_push(deq->data, el);
        ++deq->capacity;
        ++deq->end;
    }
}

static  uint32_t deq_size(const trail2d_deque_t* deq) {
    return deq->end - deq->first;
}

static void deq_clear(trail2d_deque_t* deq) {
    deq->first = 0;
    deq->end = 0;
}

static void deq_erase_front(trail2d_deque_t* deq) {
    ++deq->first;
}

static void update_trail2d_position(trail2d_t* trail, vec2_t new_position) {

    if (!trail->initialized) {
        trail->initialized = true;
        trail->last_position = new_position;
        return;
    }

    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
//    auto pos = lastPosition;
    const float distanceSqr = length_sqr_vec2(sub_vec2(new_position, trail->last_position));

//    direction *= 1.0f / distance;

//    auto headCopy = nodes.back();
    if (distanceSqr >= trail->segment_distance_max * trail->segment_distance_max) {
        trail->last_position = new_position;
        //headCopy.position = newPosition;
        trail2d_node_t new_node;
        new_node.position = vec2(0, 0);
        new_node.energy = 1;
        new_node.scale = trail->scale;
        deq_push(&trail->nodes, new_node);
    }

    if (deq_size(&trail->nodes) > 0) {
        trail2d_node_t * n = deq_back(&trail->nodes);
        n->position = new_position;
        n->scale = trail->scale;
    } else {
        //lastPosition = newPosition;
    }
}

static void update_trail2d_mat(trail2d_t* trail, const mat3x2_t m) {
    const float dt = g_time_layers[trail->timer].dt;
    const vec2_t scale2 = mat2_get_scale(m.rot);
    trail->scale = fminf(scale2.x, scale2.y);
    update_trail2d_position(trail, vec2_transform(trail->offset, m));

    for (uint32_t i = trail->nodes.first; i < trail->nodes.end; ++i) {
        trail2d_node_t * node = trail->nodes.data + i;
        node->energy -= dt * trail->drain_speed;
        if (node->energy <= 0) {
            node->energy = 0;
            if (i == trail->nodes.first) {
                deq_erase_front(&trail->nodes);
            }
        }
    }
}

void update_trail2d(void) {
    trail2d_t* trails = (trail2d_t*) Trail2D.data[0];
    const uint32_t count = Trail2D.size;
    for (uint32_t i = 1; i < count; ++i) {
        entity_idx_t ei = Trail2D.handle_to_entity[i];
        component_handle_t wti = get_component_handle_by_index(&Transform2D, ei);
        const mat3x2_t m = ((world_transform2d_t*) get_component_data(&Transform2D, wti, 1))->matrix;
        update_trail2d_mat(trails + i, m);
    }
}

static void trail_renderer2d_draw(entity_t e) {
    trail_renderer2d_t* comp = get_trail_renderer2d(e);
    trail2d_t* trail = get_trail2d(comp->target);
    trail2d_node_t * node_array = trail->nodes.data;

    const uint32_t columns = deq_size(&trail->nodes);
    if (columns < 2) {
        return;
    }
    const sprite_t* spr = &REF_RESOLVE(res_sprite, comp->sprite);
    const sg_image image = REF_RESOLVE(res_image, spr->image_id);
    if (image.id == SG_INVALID_ID) {
        return;
    }

    const uint32_t quads = columns - 1;

    canvas_set_image(image);
    canvas_triangles(columns * 2, quads * 6);

    uint32_t node_idx = trail->nodes.first;

    const color_t co = canvas.color[0].offset;
    const color_t cm = canvas.color[0].scale;
    float texCoordU0 = RECT_CENTER_X(spr->tex);
    float texCoordV0 = spr->tex.y;
    float texCoordU1 = RECT_CENTER_X(spr->tex);
    float texCoordV1 = RECT_B(spr->tex);
    if (spr->state & SPRITE_ROTATED) {
        texCoordU0 = spr->tex.x;
        texCoordV0 = RECT_CENTER_Y(spr->tex);
        texCoordU1 = RECT_R(spr->tex);
        texCoordV1 = RECT_CENTER_Y(spr->tex);
    }

    //const auto m = drawer.matrix;
    //drawer.matrix.set
    ek_vertex2d* ptr = canvas.vertex_it;

    // we could generate vertices right into destination buffer :)
    for (uint32_t i = 0; i < columns; ++i) {
        const vec2_t p = node_array[node_idx].position;
        vec2_t perp;
        if (i > 0/* node_idx > begin */) {
            perp = normalize_vec2(sub_vec2(node_array[node_idx - 1].position, p));
            if (i + 1 < columns) {
                perp = normalize_vec2(lerp_vec2(perp, normalize_vec2(sub_vec2(p, node_array[node_idx + 1].position)), 0.5f));
            }
        } else if (i + 1 < columns) {
            perp = normalize_vec2(sub_vec2(p, node_array[node_idx + 1].position));
        }
        else {
            perp = vec2(0, 0);
        }
        perp = perp_vec2(perp);

        const float energy = node_array[node_idx].energy;
        const float easedEnergy = ease_p2_out(energy);
        const float r = lerp_f32(comp->minWidth, comp->width, easedEnergy);
        const float nodeScale = node_array[node_idx].scale;
        perp = scale_vec2(perp, nodeScale * r);

        const color_t cm0 = color_alpha_scale_f(cm, energy);
        ptr->x = p.x - perp.x;
        ptr->y = p.y - perp.y;
        ptr->u = texCoordU0;
        ptr->v = texCoordV0;
        ptr->cm = cm0.value;
        ptr->co = co.value;
        ++ptr;
        ptr->x = p.x + perp.x;
        ptr->y = p.y + perp.y;
        ptr->u = texCoordU1;
        ptr->v = texCoordV1;
        ptr->cm = cm0.value;
        ptr->co = co.value;
        ++ptr;
        ++node_idx;
    }

    {
        uint16_t v = canvas.vertex_base;
        uint16_t v_end = v + quads * 2;
        EK_ASSERT(v_end >= v);
        uint16_t* indices = canvas.index_it;
        while (v != v_end) {
            *(indices++) = v;
            *(indices++) = v + 2;
            *(indices++) = v + 3;
            *(indices++) = v + 3;
            *(indices++) = v + 1;
            *(indices++) = v;
            v += 2;
        }
    }
}

trail_renderer2d_t* add_trail_renderer2d(entity_t e, entity_t target) {
    trail_renderer2d_t* r = (trail_renderer2d_t*) add_component(&TrailRenderer2D, e);
    r->target = target;
    r->width = 20;
    r->minWidth = 5;
    r->sprite = R_SPRITE_EMPTY;
    add_display2d(e)->draw = trail_renderer2d_draw;
    return r;
}
