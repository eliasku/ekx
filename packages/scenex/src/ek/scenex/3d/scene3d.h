#ifndef SCENEX_STATIC_MESH
#define SCENEX_STATIC_MESH

#include <gen_sg.h>
#include <ek/gfx.h>
#include <ecx/ecx.h>
#include <ekx/app/game_display.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    model3d_t origin;
    sg_buffer vb;
    sg_buffer ib;
    int indices_count;
} static_mesh_t;

static_mesh_t static_mesh(model3d_t model);
void static_mesh_destroy(static_mesh_t* mesh);

typedef struct {
    static_mesh_t* mesh_ptr;
    string_hash_t mesh;
    string_hash_t material;
    bool cast_shadows;// = true;
    bool receive_shadows;// = true;
} mesh_renderer_t;

struct res_mesh3d {
    string_hash_t names[32];
    static_mesh_t data[32];
    rr_man_t rr;
};

extern struct res_mesh3d res_mesh3d;

void setup_res_mesh3d(void);

#define R_MESH3D(name) REF_NAME(res_mesh3d, name)

typedef struct {
    vec3_t ambient;
    vec3_t diffuse;
    vec3_t specular;
    vec3_t emission;
    float shininess;
    float roughness;
} material3d_t;

material3d_t material3d(color_t color, float ao);

struct res_material3d {
    string_hash_t names[32];
    material3d_t data[32];
    rr_man_t rr;
};

extern struct res_material3d res_material3d;

void setup_res_material3d(void);

// light comp
enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2,
};

typedef struct {
    int32_t type;
    vec3_t ambient;
    vec3_t diffuse;
    vec3_t specular;
    float radius;
    float falloff;
    bool cast_shadows;
} light3d_t;

light3d_t light3d(int32_t light_type);

// camera

typedef struct {
    // clip plane near-far
    float z_near;
    float z_far;

    // orthogonal mode
    bool orthogonal;
    float orthogonal_size;

    // field of view in radians
    float fov;

    // camera up vector
    vec3_t up;

    bool clear_color_enabled;
    bool clear_depth_enabled;

    vec4_t clear_color;
    float clear_depth;

    R(sg_image) cubemap;
} camera3d_t;

camera3d_t camera3d(void);

typedef struct {
    mat4_t local;
    mat4_t world;
    vec3_t position;
    vec3_t scale;
    vec3_t rotation;
} transform3d_t;

void update_world_transform3d(void);

extern ecx_component_type Camera3D;
extern ecx_component_type Light3D;
extern ecx_component_type MeshRenderer;
extern ecx_component_type Transform3D;

void scene3d_setup(void);

#define get_transform3d(e) ((transform3d_t*)get_component(&Transform3D, e))
#define add_transform3d(e) ((transform3d_t*)add_component(&Transform3D, e))

#define get_mesh_renderer(e) ((mesh_renderer_t*)get_component(&MeshRenderer, e))
#define add_mesh_renderer(e) ((mesh_renderer_t*)add_component(&MeshRenderer, e))

#define get_camera3d(e) ((camera3d_t*)get_component(&Camera3D, e))
#define add_camera3d(e) ((camera3d_t*)add_component(&Camera3D, e))

#define get_light3d(e) ((light3d_t*)get_component(&Light3D, e))
#define add_light3d(e) ((light3d_t*)add_component(&Light3D, e))

/* private*/ void render_system_setup(void);

void scene3d_prerender(void);
void scene3d_render(game_display_info* display_info);

typedef struct {
    material3d_t default_material;
    mat4_t camera_projection;
    mat4_t camera_view;
    entity_t scene;
    entity_t camera;
} scene3d_render_system_t;

extern scene3d_render_system_t scene3d_render_system;

#ifdef __cplusplus
}
#endif

#endif // SCENEX_STATIC_MESH
