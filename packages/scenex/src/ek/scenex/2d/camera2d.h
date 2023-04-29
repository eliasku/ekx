#ifndef SCENEX_CAMERA2D_H
#define SCENEX_CAMERA2D_H

#include <ecx/ecx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CAMERA2D_ACTIVE_QUEUE_MAX = 16,
};

typedef struct {
    int order;

    entity_t viewportNode;
    entity_t root;
    float contentScale;

    vec2_t relativeOrigin;
    vec4_t clearColor;

    // just for fun (quad is used to clear.
    // todo: know about sync/de-sync clearing surface/RT?)
    vec4_t clearColor2;

    mat3x2_t screenToWorldMatrix;
    mat3x2_t worldToScreenMatrix;
    rect_t screenRect;
    rect_t worldRect;

    bool enabled;
    bool interactive;
    bool occlusionEnabled;
    bool clearColorEnabled;

    bool debugOcclusion;
    bool debugVisibleBounds;
    bool debugGizmoHitTarget;
    bool debugGizmoPointer;
    bool debugGizmoSelf;
    float debugDrawScale;
} camera2d_t;

camera2d_t camera2d_default(void);

extern entity_t main_camera;

mat3x2_t camera2d_calc_matrix(const camera2d_t* camera, entity_t view, float scale, vec2_t screen_offset, vec2_t screen_size);

void update_camera2d_queue(void);

void render_camera2d_queue(void);

void draw_camera2d_gizmo(camera2d_t* camera);

extern const camera2d_t* current_rendering_camera;
extern entity_t active_cameras[CAMERA2D_ACTIVE_QUEUE_MAX];
extern uint32_t active_cameras_num;

extern ECX_DEFINE_TYPE(camera2d_t);
#define Camera2D ECX_ID(camera2d_t)

void setup_camera2d(void);
#define get_camera2d(e) ECX_GET(camera2d_t,e)
#define add_camera2d(e) ECX_ADD(camera2d_t,e)

#ifdef __cplusplus
}
#endif

#endif // SCENEX_CAMERA2D_H
