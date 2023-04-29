#include "scene3d.h"

#include <ek/gfx.h>
#include "render3d_shader.h"

#include <ek/time.h>
#include <ek/canvas.h>
#include <ek/app.h>
#include <ek/scenex/base/node.h>

#include <string.h>

#undef near
#undef far

const sg_face_winding DEFAULT_FACE_WINDING = SG_FACEWINDING_CCW;

__attribute__((unused))
static aabb3_t
get_shadow_map_box(const mat4_t* camera_projection, const mat4_t* camera_view, const mat4_t* light_view) {
    UNUSED(light_view);
    const mat4_t inv_proj_view = mat4_inverse(mat4_mul(*camera_projection, *camera_view));
    const vec3_t corners[8] = {
            vec3(-1, -1, -1),
            vec3(-1, -1, 1),
            vec3(1, -1, -1),
            vec3(1, -1, 1),
            vec3(-1, 1, -1),
            vec3(-1, 1, 1),
            vec3(1, 1, -1),
            vec3(1, 1, 1),
    };
    aabb3_t bb;
    bb.min = vec3(100000, 100000, 100000);
    bb.max = vec3(-100000, -100000, -100000);

    for (uint32_t i = 0; i < 8; ++i) {
        vec4_t c;
        c.xyz = corners[i];
        c.w = 1;
        vec4_t v2 = mat4_mul_vec4(inv_proj_view, c);
        const float len = length_vec4(v2);
        vec3_t v = scale_vec3(normalize_vec3(v2.xyz), len);
        if (v.x < bb.min.x) bb.min.x = v.x;
        if (v.y < bb.min.y) bb.min.y = v.y;
        if (v.z < bb.min.z) bb.min.z = v.z;
        if (v.x > bb.max.x) bb.max.x = v.x;
        if (v.y > bb.max.y) bb.max.y = v.y;
        if (v.z > bb.max.z) bb.max.z = v.z;
    }
    return bb;
}

static sg_layout_desc get_vertex3d_layout(void) {
    sg_layout_desc desc = {0};
    desc.buffers[0].stride = sizeof(model3d_vertex_t);
    desc.attrs[0].offset = offsetof(model3d_vertex_t, position);
    desc.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    desc.attrs[1].offset = offsetof(model3d_vertex_t, normal);
    desc.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    desc.attrs[2].offset = offsetof(model3d_vertex_t, uv);
    desc.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    desc.attrs[3].offset = offsetof(model3d_vertex_t, color);
    desc.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    desc.attrs[4].offset = offsetof(model3d_vertex_t, color2);
    desc.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;
    return desc;
}

static sg_image_desc get_render_target_desc(int w, int h) {
    return (sg_image_desc) {
            .type = SG_IMAGETYPE_2D,
            .render_target = true,
            .width = w,
            .height = h,
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .sample_count = 1,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
}

struct {
    sg_image rt;
    sg_image rt_color;
    ek_shader shader;
    sg_pass pass;
    sg_pass_action clear;
    sg_pipeline pip;
    mat4_t projection;
    mat4_t view;
} shadows;

static void shadows_init(void) {
    shadows.shader = ek_shader_make(render3d_shadow_map_shader_desc(sg_query_backend()));
    const uint32_t w = 2048;
    const uint32_t h = 2048;
    sg_image_desc depth_image_desc = get_render_target_desc(w, h);
    depth_image_desc.pixel_format = SG_PIXELFORMAT_DEPTH;
    depth_image_desc.label = "shadows_depth";
    shadows.rt = sg_make_image(&depth_image_desc);
    sg_image_desc depth_color_desc = get_render_target_desc(w, h);
    depth_color_desc.label = "shadows_tex";
    shadows.rt_color = sg_make_image(&depth_color_desc);
    sg_pipeline_desc pip_desc = {0};
    pip_desc.shader = shadows.shader.shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_desc.cull_mode = SG_CULLMODE_FRONT;
    pip_desc.face_winding = DEFAULT_FACE_WINDING;
    pip_desc.sample_count = 1;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pip_desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH;
    pip_desc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
    pip_desc.layout = get_vertex3d_layout();
    pip_desc.label = "3d-shadow-map";
    shadows.pip = sg_make_pipeline(&pip_desc);

    shadows.clear.colors[0].action = SG_ACTION_CLEAR;
    shadows.clear.colors[0].value = (sg_color) {1.0f, 1.0f, 1.0f, 1.0f};
    shadows.clear.depth.action = SG_ACTION_CLEAR;
    shadows.clear.depth.value = 1.0f;

    sg_pass_desc pass_desc = {0};
    pass_desc.color_attachments[0].image = shadows.rt_color;
    pass_desc.depth_stencil_attachment.image = shadows.rt;
    pass_desc.label = "shadow-map-pass";
    shadows.pass = sg_make_pass(&pass_desc);
}

static void shadows_begin(void) {
    sg_push_debug_group("3D shadows");
    const sg_image_desc info = sg_query_image_desc(shadows.rt);
    int w = info.width;
    int h = info.height;
    sg_begin_pass(shadows.pass, &shadows.clear);
    sg_apply_viewport(0, 0, w, h, true);
    sg_apply_scissor_rect(0, 0, w, h, true);
    sg_apply_pipeline(shadows.pip);
}

static void
shadows_update_light_direction(const mat4_t* camera_projection, const mat4_t* camera_view) {
    UNUSED(camera_projection);
    UNUSED(camera_view);
// find directional light
    vec3_t light_position = vec3(0, 0, 1);
//        light3d_t light_data{};
    for (uint32_t i = 1; i < Light3D.size; ++i) {
        const entity_t e = get_entity(&Light3D, i);
        const light3d_t* l = (light3d_t*) Light3D.data[0] + i;
        const transform3d_t* transform = get_transform3d(e);
        if (transform && l->type == LIGHT_DIRECTIONAL) {
//                light_data = *l;
            light_position = normalize_vec3(mat4_get_position(&transform->world));
        }
    }

    const vec3_t light_target = vec3(0, 0, 0);
//    auto light_dir = normalize(light_target - light_position);

//auto bb = get_shadow_map_box(cameraProjection, cameraView, view);
    const float shadow_zone_size = 200.0f;
    shadows.view = mat4_look_at_rh(light_position, light_target, vec3(0, 0, 1));
    shadows.projection = mat4_orthographic_rh(-shadow_zone_size,
                                              shadow_zone_size,
                                              shadow_zone_size,
                                              -shadow_zone_size,
                                              -shadow_zone_size,
                                              shadow_zone_size);
}

static void shadows_render_objects(void) {
    sg_bindings bindings = {0};
    mat4_t mvp;

    R(StaticMesh) res_mesh = 0;
    for (uint32_t i = 1; i < MeshRenderer.size; ++i) {
        const entity_t e = get_entity(&MeshRenderer, i);
        const transform3d_t* transform = get_transform3d(e);
        const mesh_renderer_t* filter = get_mesh_renderer(e);
        if (transform && filter->cast_shadows && is_visible(e)) {
            static_mesh_t* mesh = filter->mesh_ptr;
            if (!mesh) {
                res_mesh = filter->mesh ? R_MESH3D(filter->mesh) : 0;
                mesh = res_mesh ? &REF_RESOLVE(res_mesh3d, res_mesh) : NULL;
            }
            if (mesh) {
                bindings.index_buffer = mesh->ib;
                bindings.vertex_buffers[0] = mesh->vb;
                sg_apply_bindings(&bindings);
                mvp = mat4_mul(mat4_mul(shadows.projection, shadows.view), transform->world);
                sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE_REF(mvp));
                sg_draw(0, mesh->indices_count, 1);
            }
        }
    }
}

static void shadows_end(void) {
    sg_end_pass();
    sg_pop_debug_group();
}

struct {
    ek_shader shader;
    sg_pipeline pip;
    sg_bindings bind;

    light_params_t directional_light_params;
    light2_params_t point_light_params;
} main3d;

void main3d_init(void) {
    main3d.shader = ek_shader_make(render3d_shader_desc(sg_query_backend()));

    sg_pipeline_desc desc = {0};
    desc.label = "3d-main";
    desc.shader = main3d.shader.shader;
    desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    desc.index_type = SG_INDEXTYPE_UINT16;
    desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    desc.depth.write_enabled = true;
    desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    desc.cull_mode = SG_CULLMODE_BACK;
    desc.face_winding = DEFAULT_FACE_WINDING;
    desc.sample_count = 1;
    desc.layout = get_vertex3d_layout();

    main3d.pip = sg_make_pipeline(&desc);
}

void main3d_set_directional_light_info(vec3_t pos, const light3d_t* data) {
    memcpy(main3d.directional_light_params.light_position, &pos, sizeof(vec3_t));
    memcpy(main3d.directional_light_params.light_ambient, &data->ambient, sizeof(vec3_t));
    memcpy(main3d.directional_light_params.light_diffuse, &data->diffuse, sizeof(vec3_t));
    memcpy(main3d.directional_light_params.light_specular, &data->specular, sizeof(vec3_t));
}

void main3d_set_point_light_info(vec3_t pos, const light3d_t* data) {
    memcpy(main3d.point_light_params.light2_position, pos.data, sizeof(vec3_t));
    memcpy(main3d.point_light_params.light2_ambient, data->ambient.data, sizeof(vec3_t));
    memcpy(main3d.point_light_params.light2_diffuse, data->diffuse.data, sizeof(vec3_t));
    memcpy(main3d.point_light_params.light2_specular, data->specular.data, sizeof(vec3_t));
    main3d.point_light_params.light2_radius = data->radius;
    main3d.point_light_params.light2_falloff = data->falloff;
}

struct {
    ek_shader shader;
    sg_pipeline pip;
} render_skybox;

void skybox_init(void) {
    render_skybox.shader = ek_shader_make(render3d_skybox_shader_desc(sg_query_backend()));

    sg_pipeline_desc desc = {0};
    desc.shader = render_skybox.shader.shader;
    desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    desc.index_type = SG_INDEXTYPE_UINT16;
    desc.label = "3d-skybox";
    desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    desc.depth.write_enabled = false;
    desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    desc.cull_mode = SG_CULLMODE_FRONT;
    desc.face_winding = SG_FACEWINDING_CCW;
    desc.sample_count = 1;
    desc.layout = get_vertex3d_layout();

    render_skybox.pip = sg_make_pipeline(&desc);
}

void skybox_render(const sg_image cubemap, const mat4_t* view, const mat4_t* projection) {
    static_mesh_t* mesh = &RES_NAME_RESOLVE(res_mesh3d, H("cube"));
    if (cubemap.id && mesh) {
        sg_apply_pipeline(render_skybox.pip);

        mat4_t model = mat4_identity();

        mat4_t view3 = *view;
        view3.m03 = 0;
        view3.m13 = 0;
        view3.m23 = 0;
        view3.m30 = 0;
        view3.m31 = 0;
        view3.m32 = 0;
        view3.m33 = 1;

        const mat4_t mvp = mat4_mul(mat4_mul(*projection, view3), model);

        sg_bindings bind = {0};
        bind.fs_images[0] = cubemap;
        bind.vertex_buffers[0] = mesh->vb;
        bind.index_buffer = mesh->ib;
        sg_apply_bindings(&bind);

        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE_REF(mvp));
        sg_draw(0, mesh->indices_count, 1);
    }
}

void render_system_setup(void) {
    shadows_init();
    main3d_init();
    skybox_init();
}

static void render_system_draw_objects(mat4_t proj, mat4_t view) {
    const sg_image empty = res_image.data[R_IMAGE_EMPTY];
    main3d.bind.fs_images[SLOT_uImage0] = empty;
    main3d.bind.fs_images[SLOT_u_image_shadow_map] = shadows.rt_color;

    for (uint32_t i = 1; i < MeshRenderer.size; ++i) {
        const entity_t e = get_entity(&MeshRenderer, i);
        const transform3d_t* transform = get_transform3d(e);
        const mesh_renderer_t* filter = get_mesh_renderer(e);
        static_mesh_t* mesh = filter->mesh ? &RES_NAME_RESOLVE(res_mesh3d, filter->mesh) : NULL;
        if (!mesh) mesh = filter->mesh_ptr;
        if (mesh && is_visible(e) && transform) {
            mat4_t model = transform->world;
//            mat3_t nm = mat3_transpose(mat3_inverse(mat4_get_mat3(&model)));
//            mat4_t nm4 = mat4_mat3(nm);
            mat4_t nm4 = mat4_transpose(mat4_inverse(mat4_mat3(mat4_get_mat3(&model))));

            const mat4_t depth_mvp = mat4_mul(mat4_mul(shadows.projection, shadows.view), model);

            const material3d_t material = filter->material ? RES_NAME_RESOLVE(res_material3d, filter->material)
                                                           : scene3d_render_system.default_material;
            vs_params_t params;
            memcpy(params.uModelViewProjection, mat4_mul(mat4_mul(proj, view), model).data, sizeof(mat4_t));
            memcpy(params.uModel, model.data, sizeof(mat4_t));
            memcpy(params.u_depth_mvp, depth_mvp.data, sizeof(mat4_t));
            memcpy(params.u_normal_matrix, nm4.data, sizeof(mat4_t));
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE_REF(params));

            material_params_t mat_params;
            memcpy(mat_params.mat_diffuse, material.diffuse.data, sizeof(vec3_t));
            memcpy(mat_params.mat_ambient, material.ambient.data, sizeof(vec3_t));
            memcpy(mat_params.mat_specular, material.specular.data, sizeof(vec3_t));
            memcpy(mat_params.mat_emission, material.emission.data, sizeof(vec3_t));
            mat_params.mat_shininess = (1.f / material.roughness) - 1.f;
            mat_params.mat_roughness = material.roughness;
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_material_params, SG_RANGE_REF(mat_params));

            main3d.bind.index_buffer = mesh->ib;
            main3d.bind.vertex_buffers[0] = mesh->vb;
            sg_apply_bindings(&main3d.bind);
            sg_draw(0, mesh->indices_count, 1);
        }
    }
}

static void render_system_prepare(void) {
    scene3d_render_system.default_material = material3d(RGB(0xFF00FF), 0.2f);

    if (!is_entity(scene3d_render_system.camera) ||
        !is_entity(scene3d_render_system.scene)) {
        scene3d_render_system.camera = NULL_ENTITY;
        scene3d_render_system.scene = NULL_ENTITY;
    }
    // get view camera orientation
    if (!is_entity(scene3d_render_system.camera) ||
        !is_visible(scene3d_render_system.camera)) {
        return;
    }

    const camera3d_t* camera_data = get_camera3d(scene3d_render_system.camera);
    const transform3d_t* camera_transform = get_transform3d(scene3d_render_system.camera);

    vec3_t point_light_pos = vec3(0, 15, 0);
    light3d_t point_light = {0};

    vec3_t directional_light_pos = vec3(0, 0, 1);
    light3d_t directional_light = {0};
    for (int i = 1; i < Light3D.size; ++i) {
        const entity_t e = get_entity(&Light3D, i);
        transform3d_t* transform = get_transform3d(e);
        if (transform) {
            const light3d_t* l = (light3d_t*) Light3D.data[0] + i;
            if (l->type == LIGHT_POINT) {
                point_light = *l;
                point_light_pos = mat4_get_position(&transform->world);
            } else if (l->type == LIGHT_DIRECTIONAL) {
                directional_light = *l;
                directional_light_pos = normalize_vec3(mat4_get_position(&transform->world));
            }
        }
    }

    mat4_t view = mat4_inverse(camera_transform->world);

    const float width = ek_app.viewport.width;
    const float height = ek_app.viewport.height;
    mat4_t proj;
    const float aspect = (float) width / height;
    if (camera_data->orthogonal) {
        const float ortho_size = camera_data->orthogonal_size;
        proj = mat4_orthographic_rh(-ortho_size * aspect,
                                    ortho_size * aspect,
                                    -ortho_size,
                                    ortho_size,
                                    camera_data->z_near,
                                    camera_data->z_far);
    } else {
        proj = mat4_perspective_rh(camera_data->fov, aspect, camera_data->z_near, camera_data->z_far);
    }
    scene3d_render_system.camera_projection = proj;
    scene3d_render_system.camera_view = view;
    main3d_set_directional_light_info(directional_light_pos, &directional_light);
    main3d_set_point_light_info(point_light_pos, &point_light);
}

static void render_system_prerender(void) {
    shadows_begin();
    shadows_update_light_direction(&scene3d_render_system.camera_projection, &scene3d_render_system.camera_view);
    shadows_render_objects();
    shadows_end();
}

static void render_system_render(float width, float height) {
    entity_t camera = scene3d_render_system.camera;
    /////
    // get view camera orientation
    if (!is_entity(camera) || !is_visible(camera)) {
        return;
    }

    const camera3d_t* camera_data = get_camera3d(camera);
    const transform3d_t* camera_transform = get_transform3d(camera);

    const int wi = (int) width;
    const int hi = (int) height;
    sg_apply_scissor_rect(0, 0, wi, hi, true);
    sg_apply_viewport(0, 0, wi, hi, true);

    static float fc_ = 1.0f;
    fc_ += 1.0f;
    const float time = (float) ek_time_now();

    sg_apply_pipeline(main3d.pip);

    fs_params_t fs_params;
    fs_params.u_time[0] = time;
    fs_params.u_time[1] = fract(time);
    fs_params.u_time[2] = fc_;
    fs_params.u_time[3] = 0.0f;
    fs_params.u_resolution[0] = width;
    fs_params.u_resolution[1] = height;
    fs_params.u_resolution[2] = 1.0f / width;
    fs_params.u_resolution[3] = 1.0f / height;
    const vec3_t view_pos = mat4_get_position(&camera_transform->world);
    memcpy(fs_params.uViewPos, view_pos.data, sizeof(float) * 3);

    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE_REF(fs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light_params, SG_RANGE_REF(main3d.directional_light_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light2_params, SG_RANGE_REF(main3d.point_light_params));

    render_system_draw_objects(scene3d_render_system.camera_projection, scene3d_render_system.camera_view);
    skybox_render(REF_RESOLVE(res_image, camera_data->cubemap),
                  &scene3d_render_system.camera_view,
                  &scene3d_render_system.camera_projection);
}

void scene3d_prerender(void) {
    update_world_transform3d();
    render_system_prepare();
    render_system_prerender();
}

void scene3d_render(game_display_info* display_info) {
    render_system_render(display_info->size.x, display_info->size.y);
}
