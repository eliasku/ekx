#include "render2d.h"
#include "transform2d.h"
#include "camera2d.h"
#include "display2d.h"

#include <ek/canvas.h>
#include <ek/scenex/base/node.h>

void render2d_draw(entity_t e, const world_transform2d_t* world_transform) {
    EK_ASSERT(is_entity(e));

    bounds2d_t* bounds = get_bounds2d(e);
    if (bounds) {
        const camera2d_t* camera = current_rendering_camera;
        const rect_t rc = get_screen_rect(bounds, camera->worldToScreenMatrix, world_transform->matrix);
        if (camera->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            canvas_push_scissors(rc);
        }
    }

    bool programChanged = false;
    display2d_t* display = get_display2d(e);
    if (display) {
        if (UNLIKELY(display->program)) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (LIKELY(display->draw)) {
            canvas.matrix[0] = world_transform->matrix;
            canvas.color[0] = world_transform->color;
            display->draw(e);
        }
        if(UNLIKELY(display->callback)) {
            canvas.matrix[0] = world_transform->matrix;
            canvas.color[0] = world_transform->color;
            display->callback(e);
        }
    }

    entity_t it = get_first_child(e);
    while (it.id) {
        const node_t* child = get_node(it);
        if (!(child->flags & NODE_HIDDEN)) {
            const world_transform2d_t* child_world_transform = get_world_transform2d(it);
            if (child_world_transform) {
                world_transform = child_world_transform;
            }
            if (world_transform->color.scale.a > 0) {
                render2d_draw(it, world_transform);
            }
        }
        it = child->sibling_next;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_SCISSORS)) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}

void render2d_draw_stack(entity_t e) {
    EK_ASSERT(is_entity(e));

    bounds2d_t* bounds = get_bounds2d(e);
    if (bounds) {
        const camera2d_t* camera = current_rendering_camera;
        const rect_t rc = get_screen_rect(bounds, camera->worldToScreenMatrix, canvas.matrix[0]);
        if (camera->occlusionEnabled) {
            if (!rect_overlaps(rc, camera->screenRect) || !rect_overlaps(rc, canvas.scissors[0])) {
                // discard
                return;
            }
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            canvas_push_scissors(rc);
            //draw2d::push_scissors(scissors->world_rect(transform->worldMatrix));
        }
    }

    bool programChanged = false;
    display2d_t* display = get_display2d(e);
    if (display) {
        if (display->program) {
            programChanged = true;
            canvas_push_program(REF_RESOLVE(res_shader, display->program));
        }
        if (display->draw) {
            display->draw(e);
        }
        if (display->callback) {
            display->callback(e);
        }
    }

    entity_t it = get_first_child(e);
    while (it.id) {
        const node_t* child = get_node(it);
        if (!(child->flags & NODE_HIDDEN)) {
            const transform2d_t* child_transform = get_transform2d(it);
            if (child_transform) {
                canvas_save_transform();
                canvas_concat_matrix(child_transform->matrix);
                canvas_concat_color(child_transform->color);
            }
            if (canvas.color[0].scale.a != 0) {
                render2d_draw_stack(it);
            }
            if (child_transform) {
                canvas_restore_transform();
            }
        }
        it = child->sibling_next;
    }

    if (bounds && (bounds->flags & BOUNDS_2D_SCISSORS)) {
        canvas_pop_scissors();
    }
    if (programChanged) {
        canvas_restore_program();
    }
}
