#include "camera2d.h"
#include "transform2d.h"
#include "render2d.h"
#include "viewport.h"

#include <ek/canvas.h>
#include <ek/scenex/base/node.h>

camera2d_t camera2d_default(void) {
    camera2d_t cam = INIT_ZERO;

    cam.contentScale = 1;
    cam.clearColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    cam.enabled = true;
    cam.interactive = true;
    cam.occlusionEnabled = true;
    cam.debugDrawScale = 1;

    cam.screenToWorldMatrix = mat3x2_identity();
    cam.worldToScreenMatrix = mat3x2_identity();
    cam.screenRect = rect_01();
    cam.worldRect = rect_01();

    return cam;
}

ecx_component_type Camera2D;

static void Camera2D_ctor(component_handle_t handle) {
    ((camera2d_t*) Camera2D.data[0])[handle] = camera2d_default();
}

void Camera2D_setup(void) {
    const ecx_component_type_decl decl = (ecx_component_type_decl) {
            "Camera2D",
            4,
            1,
            {sizeof(camera2d_t)}
    };
    init_component_type(&Camera2D, decl);
    Camera2D.ctor = Camera2D_ctor;
}

entity_t main_camera;

mat3x2_t
camera2d_calc_matrix(const camera2d_t* camera, entity_t view, float scale, vec2_t screen_offset, vec2_t screen_size) {
    const float inv_scale = 1.0f / (scale * camera->contentScale);
    mat3x2_t m = get_world_transform2d(view)->matrix;
    mat3x2_translate(&m, neg_vec2(mul_vec2(camera->relativeOrigin, screen_size)));
    mat3x2_scale(&m, vec2(inv_scale, inv_scale));
    mat3x2_translate(&m, neg_vec2(screen_offset));
    return m;
}


entity_t active_cameras[CAMERA2D_ACTIVE_QUEUE_MAX];
uint32_t active_cameras_num;
const camera2d_t* current_rendering_camera;
//static int current_layer_mask = 0xFF;

static int sort_active_camera(void const* a, void const* b) {
    entity_t e1 = *(entity_t*) a;
    entity_t e2 = *(entity_t*) b;
    return get_camera2d(e1)->order - get_camera2d(e2)->order;
}

void update_camera2d_queue(void) {
    active_cameras_num = 0;

    for (uint32_t i = 1; i < Camera2D.size; ++i) {
        camera2d_t* camera = (camera2d_t*) get_component_data(&Camera2D, i, 0);
        entity_t e = get_entity(&Camera2D, i);
        const viewport_t* vp = get_viewport(camera->viewportNode);
        if (!camera->enabled || !vp) {
            continue;
        }

        // maybe we need region from not 0,0 started input rect
        camera->screenRect = vp->output.screenRect;
        camera->screenToWorldMatrix = camera2d_calc_matrix(camera, e, vp->output.scale, vp->output.offset,
                                                           vp->options.baseResolution);
        camera->worldRect = rect_transform(camera->screenRect, camera->screenToWorldMatrix);

        camera->worldToScreenMatrix = camera->screenToWorldMatrix;
        if (mat3x2_inverse(&camera->worldToScreenMatrix)) {
            active_cameras[active_cameras_num++] = e;
            EK_ASSERT(active_cameras_num <= CAMERA2D_ACTIVE_QUEUE_MAX);
        } else {
            // please debug camera setup
            EK_ASSERT(false);
        }
    }
    qsort(active_cameras, active_cameras_num, sizeof(entity_t), sort_active_camera);
}

void render_camera2d_queue(void) {
    for (uint32_t i = 0; i < active_cameras_num; ++i) {
        const entity_t e = active_cameras[i];
        if (!is_entity(e)) {
            continue;
        }
        camera2d_t* camera = get_camera2d(e);
        if (rect_is_empty(camera->screenRect)) {
            continue;
        }

        // set current
        current_rendering_camera = camera;

        sg_push_debug_group("Camera");
        canvas_begin_ex(camera->screenRect, camera->worldToScreenMatrix, (sg_image){0}, (sg_image){0});
        sg_apply_viewportf(camera->screenRect.x, camera->screenRect.y, camera->screenRect.w, camera->screenRect.h,
                           true);
        if (camera->clearColorEnabled) {
            canvas_push_program(res_shader.data[R_SHADER_SOLID_COLOR]);
            canvas.color[0] = (color2_t){{color_vec4(camera->clearColor), color_vec4(camera->clearColor2)}};
            canvas_fill_rect(camera->worldRect, COLOR_WHITE);
            canvas.color[0] = color2_identity();
            canvas_restore_program();
        }

        entity_t camera_root = camera->root;
        if (is_entity(camera_root) && is_visible(camera_root)) {
            render2d_draw(camera_root, get_world_transform2d(camera_root));
        }

#ifndef NDEBUG
        //        draw2d::begin(camera.screenRect, camera.worldToScreenMatrix);
        draw_camera2d_gizmo(camera);
        //        sg_apply_viewportf(camera.screenRect.x, camera.screenRect.y, camera.screenRect.width, camera.screenRect.height, true);
#endif

        canvas_end();
        sg_pop_debug_group();
    }
}

