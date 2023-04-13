#include "hit_test.h"

#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/base/node.h>

static entity_t hit_test_2d_n(entity_t e, const node_t* node, vec2_t parentPosition) {
    if (node->flags & (NODE_UNTOUCHABLE | NODE_HIDDEN)) {
        return NULL_ENTITY;
    }

    vec2_t local = parentPosition;
    const transform2d_t* transform = get_transform2d(e);
    if (LIKELY(transform)) {
        const bool ok = vec2_transform_inverse(local, transform->matrix, &local);
        // TODO: unlikely
        if (UNLIKELY(!ok)) {
            // invalid transform, break
            return NULL_ENTITY;
        }
    }

    bounds2d_t* bounds = get_bounds2d(e);
    if (bounds && !rect_contains(bounds->rect, local)) {
        return NULL_ENTITY;
    }

    entity_t it = node->child_last;
    while (it.id) {
        const node_t* child_node = Node_get(it);
        const entity_t hit = hit_test_2d_n(it, child_node, local);
        if (hit.id) {
            return hit;
        }
        it = child_node->sibling_prev;
    }

    const display2d_t* display = get_display2d(e);
    if (display && display->hit_test && display->hit_test(e, local)) {
        return e;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_HIT_AREA) && rect_contains(bounds->rect, local)) {
        return e;
    }

    return NULL_ENTITY;
}

entity_t hit_test_2d(entity_t e, vec2_t parentPosition) {
    return hit_test_2d_n(e, Node_get(e), parentPosition);
}
