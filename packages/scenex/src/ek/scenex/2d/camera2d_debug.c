#include <ek/canvas.h>
#include <ek/scenex/interaction_system.h>

#include <ek/scenex/base/node.h>
#include <ekx/app/time_layers.h>
#include "camera2d.h"
#include "viewport.h"
#include "transform2d.h"
#include "display2d.h"

static void debug_draw_pointer(camera2d_t* camera) {
    const vec2_t v = vec2_transform(
            g_interaction_system.pointerScreenPosition_,
            camera->screenToWorldMatrix
    );
    const float t = g_time_layers[TIME_LAYER_ROOT].total;
    canvas_set_empty_image();
    if (g_interaction_system.pointerDown_) {
        canvas_fill_circle(vec3_v(v, 12 + 2 * sinf(t * 8)), ARGB(0x00FFFF00), ARGB(0x77FF0000), 10);
    } else {
        canvas_fill_circle(vec3_v(v, 12 + 2 * sinf(t)), ARGB(0x0), ARGB(0x77FFFFFF), 10);
    }
}

static void
debug_draw_box(const rect_t rc, const mat3x2_t m, color_t color1, color_t color2, bool cross, color_t fillColor) {

    canvas_set_empty_image();
    if (fillColor.value) {
        canvas_save_matrix();
        canvas.matrix[0] = m;
        canvas_fill_rect(rc, fillColor);
        canvas_restore_matrix();
    }
    const aabb2_t bb = aabb2_from_rect(rc);
    const vec2_t v1 = vec2_transform(bb.min, m);
    const vec2_t v2 = vec2_transform(vec2(bb.x1, bb.y0), m);
    const vec2_t v3 = vec2_transform(bb.max, m);
    const vec2_t v4 = vec2_transform(vec2(bb.x0, bb.y1), m);
    canvas_line_ex(v1, v2, color1, color2, 2, 1);
    canvas_line_ex(v2, v3, color1, color2, 1, 2);
    canvas_line_ex(v3, v4, color1, color2, 2, 1);
    canvas_line_ex(v4, v1, color1, color2, 1, 2);
    if (cross) {
        canvas_line_ex(v1, v3, color1, color2, 2, 2);
        canvas_line_ex(v2, v4, color1, color2, 1, 1);
    }
}

static void debug_draw_hit_target(camera2d_t* camera) {
    (void) camera;
    entity_t target = g_interaction_system.hitTarget_;
    if (!is_entity(target)) {
        return;
    }
    mat3x2_t matrix = mat3x2_identity();
    transform2d_t* worldTransform = (transform2d_t*) find_component_in_parent(&Transform2D, target, 0);
    if (worldTransform) {
        matrix = worldTransform->matrix;
    }
    display2d_t* display = get_display2d(target);
    if (display && display->get_bounds) {
        debug_draw_box(display->get_bounds(target), matrix, COLOR_BLACK, COLOR_WHITE, true, COLOR_ZERO);
    }
    bounds2d_t* bounds = get_bounds2d(target);
    if (bounds) {
        if (bounds->flags & BOUNDS_2D_HIT_AREA) {
            debug_draw_box(bounds->rect, matrix, RGB(0x99FF00), COLOR_WHITE, false, COLOR_ZERO);
        }
        if (bounds->flags & BOUNDS_2D_SCISSORS) {
            debug_draw_box(bounds->rect, matrix, RGB(0xFFFF00), COLOR_WHITE, false, COLOR_ZERO);
        }
    }
}

static void traverse_visible_nodes(entity_t e,
                                 const world_transform2d_t* parentTransform,
                                 void(* callback)(entity_t e, void* component, const world_transform2d_t* transform),
                                 ecx_component_type* component_type) {
    if (!is_visible(e)) {
        return;
    }

    world_transform2d_t* transform = get_world_transform2d(e);
    if (transform) {
        if (transform->color.scale.a <= 0.0) {
            return;
        }
        parentTransform = transform;
    }

    void* component = get_component(component_type, e);
    if (component) {
        callback(e, component, parentTransform);
    }

    entity_t it = get_first_child(e);
    while (it.id) {
        traverse_visible_nodes(it, parentTransform, callback, component_type);
        it = get_next_child(it);
    }
}

static void on_debug_node_display_bounds(entity_t e, void* component, const world_transform2d_t* transform) {
    display2d_t* display = (display2d_t*) component;
    if (display->get_bounds) {
        const rect_t rc = display->get_bounds(e);
        canvas_fill_rect(rc, ARGB(0x33FFFFFF));
        debug_draw_box(rc, transform->matrix, COLOR_BLACK, COLOR_WHITE,
                       false, ARGB(0x33FFFFFF));
    } else {
        const vec2_t v = vec2_transform(vec2(0, 0), transform->matrix);
        canvas_fill_circle(vec3_v(v, 20.0f), ARGB(0xFFFF0000),
                           ARGB(0x77FF0000), 7);
    }
}

static void on_debug_node_bounds(entity_t e, void* data, const world_transform2d_t* transform) {
    UNUSED(e);
    bounds2d_t* bounds = (bounds2d_t*) data;
    if (bounds->flags & BOUNDS_2D_SCISSORS) {
        debug_draw_box(bounds->rect, transform->matrix, ARGB(0xFFFFFF00), COLOR_WHITE, true,
                       ARGB(0x55FFFF00));
    } else if (bounds->flags & BOUNDS_2D_HIT_AREA) {
        debug_draw_box(bounds->rect, transform->matrix, ARGB(0xFF00FF00), COLOR_WHITE, true,
                       ARGB(0x5500FF00));
    } else if (bounds->flags & BOUNDS_2D_CULL) {
        debug_draw_box(bounds->rect, transform->matrix, ARGB(0xFF00FFFF), COLOR_WHITE, true,
                       ARGB(0x5500FFFF));
    }
}

static void debug_camera_fills(camera2d_t* camera) {
    canvas_save_transform();
    canvas.color[0] = color2_identity();
    canvas_set_empty_image();

    traverse_visible_nodes(camera->root, NULL, on_debug_node_display_bounds, &Display2D);
    traverse_visible_nodes(camera->root, NULL, on_debug_node_bounds, &Bounds2D);

    canvas_restore_transform();
}

static rect_t ctx_occlusion_camera_rect;

static void on_debug_node_occlusion_bounds(entity_t e, void* data,
                                           const world_transform2d_t* transform) {
    UNUSED(e);
    bounds2d_t* bounds = (bounds2d_t*) data;
    const rect_t worldRect = rect_transform(bounds->rect, transform->matrix);
    const bool occluded = !rect_overlaps(worldRect, ctx_occlusion_camera_rect);
    const color_t worldColor = occluded ? ARGB(0x77FF0000) : ARGB(0x7700FF00);
    debug_draw_box(worldRect, mat3x2_identity(), worldColor, worldColor, false,
                   COLOR_ZERO);
    const color_t boundsColor = occluded ? ARGB(0x77770000) : ARGB(0x77007700);
    debug_draw_box(bounds->rect, transform->matrix, boundsColor, boundsColor, false,
                   COLOR_ZERO);
}

static void debug_occlusion(camera2d_t* camera) {
    canvas_save_transform();
    canvas.color[0] = color2_identity();
    canvas_set_empty_image();

    ctx_occlusion_camera_rect = camera->worldRect;
    traverse_visible_nodes(camera->root, NULL, on_debug_node_occlusion_bounds, &Bounds2D);

    canvas_restore_transform();
}

static void debug_camera_gizmo(camera2d_t* camera) {
    const rect_t rc = rect_expand(camera->worldRect, -10.0f);
    debug_draw_box(rc, mat3x2_identity(), COLOR_WHITE, COLOR_BLACK, false, COLOR_ZERO);

    {
        // it's not correct because:
        // - viewport's viewport is applied
        // - viewport's MVP matrix
        // TODO: make display-space debug drawing for all viewports via inspector
        const viewport_t* vp = get_viewport(camera->viewportNode);
        if (vp) {
            canvas_fill_rect(vp->output.safeRect, ARGB(0x77FF00FF));
            canvas_quad_color(0, 0, vp->options.baseResolution.x, vp->options.baseResolution.y, ARGB(0x7700FFFF));
        }
    }
    const vec2_t v = vec2_transform(
            add_vec2(camera->screenRect.position, mul_vec2(camera->relativeOrigin, camera->screenRect.size)),
            camera->screenToWorldMatrix
    );
    canvas_fill_circle(vec3_v(v, 10.0f), ARGB(0x00FFFFFF), ARGB(0x44FFFFFF), 7);
    canvas_line_ex(sub_vec2(v, vec2(20, 0)), add_vec2(v, vec2(20, 0)), COLOR_BLACK, COLOR_WHITE, 1, 3);
    canvas_line_ex(sub_vec2(v, vec2(0, 20)), add_vec2(v, vec2(0, 20)), COLOR_BLACK, COLOR_WHITE, 3, 1);
}

void draw_camera2d_gizmo(camera2d_t* camera) {

    canvas.matrix[0] = mat3x2_identity();
    canvas.color[0] = color2_identity();

    if (camera->debugVisibleBounds) {
        debug_camera_fills(camera);
    }
    if (camera->debugOcclusion) {
        debug_occlusion(camera);
    }
    if (camera->debugGizmoHitTarget) {
        debug_draw_hit_target(camera);
    }
    if (camera->debugGizmoPointer) {
        debug_draw_pointer(camera);
    }
    if (camera->debugGizmoSelf) {
        canvas.matrix[0] = mat3x2_identity();
        canvas.color[0] = color2_identity();
        debug_camera_gizmo(camera);
    }
}
