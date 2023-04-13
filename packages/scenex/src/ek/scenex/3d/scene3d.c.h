#include "scene3d.h"
#include <ecx/ecx.h>
#include <ek/gfx.h>
#include <ek/buf.h>
#include <ek/scenex/base/node.h>
#include "render_system.c.h"

ecx_component_type MeshRenderer;
ecx_component_type Transform3D;
ecx_component_type Camera3D;
ecx_component_type Light3D;
scene3d_render_system_t scene3d_render_system;

static void Transform3D_ctor(component_handle_t handle) {
    ((transform3d_t*) Transform3D.data[0])[handle] = (transform3d_t) {
            .local = mat4_identity(),
            .world = mat4_identity(),
            .position = {0},
            .scale = vec3(1, 1, 1),
            .rotation = {0},
    };
}

static void MeshRenderer_ctor(component_handle_t handle) {
    ((mesh_renderer_t*) MeshRenderer.data[0])[handle] = (mesh_renderer_t) {
            .mesh_ptr = NULL,
            .mesh = 0,
            .material = 0,
            .cast_shadows = true,
            .receive_shadows = true,
    };
}

void scene3d_setup(void) {
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "MeshRenderer",
                16,
                1,
                {sizeof(mesh_renderer_t)}
        };
        init_component_type(&MeshRenderer, decl);
        MeshRenderer.ctor = MeshRenderer_ctor;
    }
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "Transform3D",
                64,
                1,
                {sizeof(transform3d_t)}
        };
        init_component_type(&Transform3D, decl);
        Transform3D.ctor = Transform3D_ctor;
    }
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "Camera3D",
                4,
                1,
                {sizeof(camera3d_t)}
        };
        init_component_type(&Camera3D, decl);
    }
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "Light3D",
                8,
                1,
                {sizeof(light3d_t)}
        };
        init_component_type(&Light3D, decl);
    }
    setup_res_material3d();
    setup_res_mesh3d();

    scene3d_render_system.camera_projection = mat4_identity();
    scene3d_render_system.camera_view = mat4_identity();

    render_system_setup();
}

static_mesh_t static_mesh(model3d_t model) {
    static_mesh_t mesh;

    mesh.origin = model;

    sg_buffer_desc desc = {0};
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
    desc.data.ptr = model.vertices;
    desc.data.size = arr_size(model.vertices) * sizeof(model3d_vertex_t);
    mesh.vb = sg_make_buffer(&desc);
    EK_ASSERT(mesh.vb.id != 0);

    uint32_t num_indices = arr_size(model.indices);
    desc.usage = SG_USAGE_IMMUTABLE;
    desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    desc.data.ptr = model.indices;
    desc.data.size = num_indices * sizeof(uint16_t);
    mesh.ib = sg_make_buffer(&desc);
    EK_ASSERT(mesh.ib.id != 0);

    mesh.indices_count = (int) num_indices;
    return mesh;
}

void static_mesh_destroy(static_mesh_t* mesh) {
    if (mesh->vb.id != SG_INVALID_ID) {
        sg_destroy_buffer(mesh->vb);
        mesh->vb = (sg_buffer) {SG_INVALID_ID};
    }
    if (mesh->ib.id != SG_INVALID_ID) {
        sg_destroy_buffer(mesh->ib);
        mesh->ib = (sg_buffer) {SG_INVALID_ID};
    }
    if (mesh->origin.vertices) {
        free(mesh->origin.vertices);
        mesh->origin.vertices = NULL;
    }
    if (mesh->origin.indices) {
        free(mesh->origin.indices);
        mesh->origin.indices = NULL;
    }
    mesh->indices_count = 0;
}

material3d_t material3d(color_t color, float ao) {
    const float k = 0.8f;
    const vec3_t diffuse = vec4_color(color).xyz;
    return (material3d_t) {
            .ambient = scale_vec3(diffuse, ao),
            .diffuse = diffuse,
            .specular = vec3(k, k, k),
            .emission = vec3(0, 0, 0),
            .shininess = 32.0f,
            .roughness = 0.05f,
    };
}

struct res_material3d res_material3d;
struct res_mesh3d res_mesh3d;

void setup_res_material3d(void) {
    struct res_material3d* R = &res_material3d;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void setup_res_mesh3d(void) {
    struct res_mesh3d* R = &res_mesh3d;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

light3d_t light3d(int32_t light_type) {
    return (light3d_t) {
            .type = light_type,

            .ambient = vec3(0.1f, 0.1f, 0.1f),
            .diffuse = vec3(1, 1, 1),
            .specular = vec3(1, 1, 1),

            .radius = 100.0f,
            .falloff = 1.0f,

            .cast_shadows = true,
    };
}

camera3d_t camera3d(void) {
    return (camera3d_t) {
            .z_near = 10.0f,
            .z_far = 1000.0f,
            .orthogonal = false,
            .orthogonal_size = 30.0f,
            .fov = to_radians(45.0f),
            .up = vec3(0, 0, 1),
            .clear_color_enabled = true,
            .clear_depth_enabled = true,
            .clear_color = vec4(0.5f, 0.5f, 0.5f, 1.0f),
            .clear_depth = 1.0f,
            .cubemap = 0,
    };
}

static void update_world_matrix3d(entity_t e, const mat4_t* parent) {
    transform3d_t* tr = get_transform3d(e);
    if (tr) {
        tr->world = mat4_mul(tr->local, *parent);
        parent = &tr->world;
    }
    node_t* node = Node_get(e);
    if (node) {
        entity_t it = node->child_first;
        while (it.id) {
            update_world_matrix3d(it, parent);
            it = get_next_child(it);
        }
    }
}

void update_world_transform3d(void) {
    // TODO: correct hierarchy update
    for (uint32_t i = 1; i < Transform3D.size; ++i) {
        transform3d_t* t = (transform3d_t*) Transform3D.data[0] + i;
        t->local = //mat4_translate_transform(tr.position);
                mat4_mul(
                        mat4_mul(
                                mat4_translate_transform(t->position),
                                mat4_rotation_transform_quat(
                                        normalize_quat(quat_euler_angles(t->rotation))
                                )
                        ),
                        mat4_scale_transform(t->scale)
                );
    }
    const mat4_t identity = mat4_identity();
    for (uint32_t i = 1; i < Transform3D.size; ++i) {
        const entity_t e = get_entity(&Transform3D, i);
        const node_t* node = Node_get(e);
        if (!node || !node->parent.id) {
            update_world_matrix3d(e, &identity);
        }
    }
}
