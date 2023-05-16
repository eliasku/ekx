#pragma once

#include "widgets.h"
#include <ecx/ecx.h>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/camera2d.h>
#include <ek/scenex/2d/layout_rect.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/text2d.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/2d/viewport.h>
#include <ek/scenex/3d/scene3d.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/base/node_events.h>
#include <ek/scenex/particles/particle_system.h>
#include <ek/scenex/text/font.h>

PodArray<entity_id_t> inspector_list = {};

static void gui_inspector(entity_t e);

void draw_inspector_window(void) {
    if (!inspector_list.empty()) {
        if (inspector_list.size() > 1) {
            ImGui::LabelText("Multiple Selection", "%u", inspector_list.size());
        } else {
            gui_inspector(entity_id(inspector_list[0]));
        }
    }
}

inline void select_ref_asset(const char* label, rr_man_t* man, res_id* sid) {
    if (ImGui::BeginCombo(label, hsp_get(man->names[*sid]))) {
        // TODO:
        for (res_id i = 0; i < man->num; ++i) {
            string_hash_t name_hash = man->names[i];
            if (name_hash != 0) {
                const char* name = hsp_get(name_hash);
                if (ImGui::Selectable(name, i == *sid)) {
                    *sid = i;
                }
            }
        }
        ImGui::EndCombo();
    }
}

inline void guiEntityRef(const char* label, entity_t e) {
    if (e.id == 0) {
        ImGui_TextDisabled("%s: null", label);
    } else if (!is_entity(e)) {
        ImGui_TextError("%s: invalid", label);
    } else {
        auto tag = get_tag(e);
        ImGui::LabelText(label, "%s [%08X]", hsp_get(tag), tag);
    }
}

inline void guiComponentPanel(const char* name, void* data, void (*fn)(void* data)) {
    if (data && ImGui::CollapsingHeader(name)) {
        ImGui_PushID((uintptr_t)data);
        ImGui::Indent();
        fn(data);
        ImGui::Unindent();
        ImGui_PopID();
    }
}

inline void guiMovieClip(void* comp) {
    movieclip_t* mc = (movieclip_t*)comp;
    const sg_movie_t* data = mc->data;
    if (data) {
        ImGui::LabelText("Total Frames", "%u", data->frames);
        ImGui::LabelText("Default FPS", "%f", data->fps);
        ImGui::DragFloat("Time", &mc->time, 1.0f, 0.0f, (float)data->frames);
        ImGui::DragFloat("FPS", &mc->fps, 1.0f, 0.0f, 100.0f);
        ImGui::Checkbox("Playing", &mc->playing);
    }
}

inline void guiTransform2D(void* comp) {
    transform2d_t* transform = (transform2d_t*)comp;
    auto pos = transform->pos;
    auto scale = transform->cached_scale;
    auto skew = transform->cached_skew;
    if (ImGui::DragFloat2("Position", pos.data, 1.0f, 0.0f, 0.0f, "%.1f")) {
        transform->pos = pos;
    }
    if (ImGui::DragFloat2("Scale", scale.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform2d_set_scale(transform, scale);
    }
    if (ImGui::DragFloat2("Skew", skew.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform2d_set_skew(transform, skew);
    }
    //    ImGui::DragFloat2("Origin", transform.origin.data(), 0.1f, 0.0f, 0.0f, "%.2f");
    //    ImGui::DragFloat2("Pivot", transform.pivot.data(), 0.1f, 0.0f, 0.0f, "%.2f");

    auto color = vec4_color(transform->color.scale);
    if (ImGui::ColorEdit4("Color Scale", color.data)) {
        transform->color.scale = color_vec4(color);
    }

    color = vec4_color(transform->color.offset);
    if (ImGui::ColorEdit4("Color Offset", color.data)) {
        transform->color.offset = color_vec4(color);
    }
}

inline void guiViewport(void* comp) {
    viewport_t* vp = (viewport_t*)comp;
    ImGui_EditRect("Viewport", &vp->options.viewport);
    ImGui::DragFloat2("Alignment", vp->options.alignment.data);
    ImGui::DragFloat2("Base Resolution", vp->options.baseResolution.data);
    ImGui::DragFloat2("Pixel Ratio", vp->options.pixelRatio.data);
    ImGui::DragFloat2("Safe Area Fit", vp->options.safeAreaFit.data);
    ImGui::Checkbox("Scale To Resolution", &vp->options.scaleToResolution);
    if (ImGui::CollapsingHeader("Debug Output")) {
        ImGui::Indent();
        ImGui_EditRect("Screen Rect", &vp->output.screenRect);
        ImGui_EditRect("Full Rect", &vp->output.fullRect);
        ImGui_EditRect("Safe Rect", &vp->output.safeRect);
        ImGui::InputFloat2("Offset", vp->output.offset.data);
        ImGui::InputFloat("Scale", &vp->output.scale);
        ImGui::Unindent();
    }
}

inline void guiCamera2D(void* comp) {
    camera2d_t* camera = (camera2d_t*)comp;
    ImGui::Checkbox("Enabled", &camera->enabled);
    ImGui::Checkbox("interactive", &camera->interactive);
    ImGui::Checkbox("occlusionEnabled", &camera->occlusionEnabled);
    ImGui::DragInt("Order", &camera->order);
    guiEntityRef("Root Entity", camera->root);
    ImGui::DragFloat("Content Scale", &camera->contentScale);
    ImGui::Checkbox("Clear Color", &camera->clearColorEnabled);
    ImGui::ColorEdit4("Clear Color", camera->clearColor.data);
    ImGui::ColorEdit4("Clear Color+", camera->clearColor2.data);
    //ImGui::EditRect("viewport", camera.viewport.data());
    ImGui::DragFloat2("relativeOrigin", camera->relativeOrigin.data);

    ImGui_Separator();
    ImGui::Checkbox("Draw Occlusion", &camera->debugOcclusion);
    ImGui::Checkbox("Draw Bounds", &camera->debugVisibleBounds);
    ImGui::Checkbox("Draw Hit Target", &camera->debugGizmoHitTarget);

    ImGui::Checkbox("Draw Pointer", &camera->debugGizmoPointer);
    ImGui::Checkbox("Draw Camera Gizmo", &camera->debugGizmoSelf);
    ImGui::DragFloat("Debug Scale", &camera->debugDrawScale);
}

inline void guiTransform3D(void* comp) {
    transform3d_t* transform = (transform3d_t*)comp;
    ImGui::DragFloat3("Position", transform->position.data, 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat3("Scale", transform->scale.data, 0.1f, 0.0f, 0.0f, "%.2f");
    vec3_t euler_angles = scale_vec3(transform->rotation, 180.0f / MATH_PI);
    if (ImGui::DragFloat3("Rotation", euler_angles.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform->rotation = scale_vec3(euler_angles, MATH_PI / 180.0f);
    }
}

inline void guiCamera3D(void* comp) {
    camera3d_t* camera = (camera3d_t*)comp;
    ImGui::DragFloatRange2("Clip Plane", &camera->z_near, &camera->z_far, 1.0f, 0.0f, 0.0f, "%.1f");
    float fov_degree = to_degrees(camera->fov);
    if (ImGui::DragFloat("FOV", &fov_degree, 1.0f, 0.0f, 0.0f, "%.1f")) {
        camera->fov = to_radians(fov_degree);
    }

    ImGui::Checkbox("Orthogonal", &camera->orthogonal);
    ImGui::DragFloat("Ortho Size", &camera->orthogonal_size, 1.0f, 0.0f, 0.0f, "%.1f");

    ImGui::Checkbox("Clear Color Enabled", &camera->clear_color_enabled);
    ImGui::ColorEdit4("Clear Color", camera->clear_color.data);
    ImGui::Checkbox("Clear Depth Enabled", &camera->clear_depth_enabled);
    ImGui::DragFloat("Clear Depth", &camera->clear_depth, 1.0f, 0.0f, 0.0f, "%.1f");
}

inline void guiMeshRenderer(void* comp) {
    mesh_renderer_t* renderer = (mesh_renderer_t*)comp;
    ImGui::Checkbox("Cast Shadows", &renderer->cast_shadows);
    ImGui::Checkbox("Receive Shadows", &renderer->receive_shadows);
}

inline void guiLight3D(void* comp) {
    light3d_t* light = (light3d_t*)comp;
    if (light->type == LIGHT_DIRECTIONAL) {
        ImGui_Text("Directional Light");
    } else if (light->type == LIGHT_POINT) {
        ImGui_Text("Point Light");
    } else if (light->type == LIGHT_SPOT) {
        ImGui_Text("Spot Light");
    }
    ImGui::ColorEdit3("Ambient", light->ambient.data);
    ImGui::ColorEdit3("Diffuse", light->diffuse.data);
    ImGui::ColorEdit3("Specular", light->specular.data);

    ImGui::DragFloat("Radius", &light->radius, 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat("Falloff", &light->falloff, 0.1f, 0.0f, 0.0f, "%.1f");
}

inline void guiBounds2D(void* comp) {
    bounds2d_t* bounds = (bounds2d_t*)comp;
    ImGui_EditRect("Rect", &bounds->rect);
    ImGui::CheckboxFlags("Hit Area", &bounds->flags, BOUNDS_2D_HIT_AREA);
    ImGui::CheckboxFlags("Scissors", &bounds->flags, BOUNDS_2D_SCISSORS);
    ImGui::CheckboxFlags("Cull Box", &bounds->flags, BOUNDS_2D_CULL);
}

inline void gui_interactive(void* comp) {
    interactive_t* inter = (interactive_t*)comp;
    ImGui::Checkbox("pushed", &inter->pushed);
    ImGui::Checkbox("over", &inter->over);
    ImGui::Checkbox("bubble", &inter->bubble);
    ImGui::LabelText("cursor", inter->cursor == EK_MOUSE_CURSOR_BUTTON ? "button" : "?");
}

inline void gui_node_events(void* comp) {
    ImGui::LabelText("Listeners", "TODO: count");
}

inline void spriteRefInfo(R(sprite_t) ref) {
    sprite_t spr = REF_RESOLVE(res_sprite, ref);
    ImGui::LabelText("Image", "ref: %u", spr.image_id);
    ImGui::LabelText("Loaded", "%u", !!(spr.state & SPRITE_LOADED));
    ImGui::LabelText("Rotated", "%u", !!(spr.state & SPRITE_ROTATED));
    ImGui_TextDisabled("TODO: select sprite res %u", ref);
}

inline void editDisplaySprite(void* comp) {
    sprite2d_t* sprite = (sprite2d_t*)comp;
    spriteRefInfo(sprite->src);
    ImGui::Checkbox("Hit Pixels", &sprite->hit_pixels);
}

inline void editDisplayNinePatch(void* comp) {
    ninepatch2d_t* ninepatch = (ninepatch2d_t*)comp;
    spriteRefInfo(ninepatch->src);
    // TODO: scale, size
    //ImGui::Checkbox("Scale Grid", &ninePatch.scale_grid_mode);
    ImGui::Checkbox("Hit Pixels", &ninepatch->hit_pixels);
}

inline void editDisplayRectangle(void* comp) {
    quad2d_t* quad = (quad2d_t*)comp;
    ImGui_EditRect("Bounds", &quad->rect);
    ImGui_Color32Edit("Color LT", &quad->colors[0]);
    ImGui_Color32Edit("Color RT", &quad->colors[1]);
    ImGui_Color32Edit("Color RB", &quad->colors[2]);
    ImGui_Color32Edit("Color LB", &quad->colors[3]);
}

inline void editDisplayArc(void* comp) {
    arc2d_t* arc = (arc2d_t*)comp;
    spriteRefInfo(arc->sprite);
    ImGui::DragFloat("Angle", &arc->angle);
    ImGui::DragFloat("Radius", &arc->radius);
    ImGui::DragFloat("Line Width", &arc->line_width);
    ImGui::DragInt("Segments", &arc->segments);
    ImGui_Color32Edit("Color Inner", &arc->color_inner);
    ImGui_Color32Edit("Color Outer", &arc->color_outer);
}

inline void editParticleRenderer2D(void* comp) {
    particle_renderer2d_t* p = (particle_renderer2d_t*)comp;
    guiEntityRef("Target", p->target);
}

inline void guiTextFormat(text_format_t* format) {
    select_ref_asset("Font", &res_font.rr, &format->font);
    ImGui::DragFloat("Size", &format->size, 1, 8, 128, "%f");
    ImGui::DragFloat("Leading", &format->leading, 1, 0, 128, "%f");
    ImGui::DragFloat("Spacing", &format->letterSpacing, 1, -128, 128, "%f");
    ImGui::DragFloat2("Alignment", format->alignment.data, 0.5f, 0.0f, 1.0f);
    ImGui::Checkbox("Kerning", &format->kerning);
    ImGui::Checkbox("Underline", &format->underline);
    ImGui::Checkbox("Strikethrough", &format->strikethrough);

    ImGui::SliderInt("Layers", &format->layersCount, 0, TEXT_LAYERS_MAX);
    if (ImGui::Checkbox("Show all glyph bounds", &format->layers[0].showGlyphBounds)) {
        bool sgb = format->layers[0].showGlyphBounds;
        for (int i = 0; i < TEXT_LAYERS_MAX; ++i) {
            format->layers[i].showGlyphBounds = sgb;
        }
    }

    ImGui::Indent();
    for (int i = 0; i < format->layersCount; ++i) {
        text_layer_effect_t* layer = &format->layers[i];
        gui_text_layer_effect(layer);
    }
    ImGui::Unindent();
}

inline void editDisplayText(void* comp) {
    text2d_t* tf = (text2d_t*)comp;
    // TODO:
    ImGui::LabelText("Text", "%s", text2d__c_str(tf));
    switch (tf->flags & TEXT2D_STR_MASK) {
        case TEXT2D_C_STR:
            ImGui::LabelText("Buffer Mode", "c-string");
            break;
        case TEXT2D_INPLACE:
            ImGui::LabelText("Buffer Mode", "in-place");
            break;
        case TEXT2D_STR_BUF:
            ImGui::LabelText("Buffer Mode", "dynamic");
            ImGui_InputTextMultiline("Text", &tf->str_obj, vec2(0,0), 0, NULL, NULL);
            break;
    }
    ImGui_EditRect("Bounds", &tf->rect);
    ImGui_Color32Edit("Border Color", &tf->borderColor);
    ImGui_Color32Edit("Fill Color", &tf->fillColor);
    ImGui::Checkbox("Hit Text Bounds", &tf->hitTextBounds);
    ImGui::Checkbox("Show Text Bounds", &tf->showTextBounds);
    guiTextFormat(&tf->format);
}

inline void guiLayout(void* comp) {
    layout_rect_t* layout = (layout_rect_t*)comp;
    ImGui::Checkbox("Fill X", &layout->fill_x);
    ImGui::Checkbox("Fill Y", &layout->fill_y);
    ImGui::Checkbox("Align X", &layout->align_x);
    ImGui::Checkbox("Align Y", &layout->align_y);
    ImGui::Checkbox("Use Safe Insets", &layout->doSafeInsets);
    ImGui_EditRect("Extra Fill", &layout->fill_extra);
    ImGui::DragFloat2("Align X (rel, abs)", layout->x.data);
    ImGui::DragFloat2("Align Y (rel, abs)", layout->y.data);

    ImGui_EditRect("Rect", &layout->rect);
    ImGui_EditRect("Safe Rect", &layout->safeRect);
}

inline void guiParticleEmitter2D(void* comp) {
    particle_emitter2d_t* emitter = (particle_emitter2d_t*)comp;
    ImGui::Checkbox("Enabled", &emitter->enabled);
    ImGui_Text("_Time: %f", emitter->time);
    guiEntityRef("Layer", emitter->layer);
    ImGui::LabelText("Particle", "SID: %u ( %s )", emitter->particle, hsp_get(res_particle.names[emitter->particle]));
    ImGui::DragFloat2("Offset", emitter->position.data);
    ImGui_Separator();

    // data
    ImGui_EditRect("Rect", &emitter->data.rect);
    ImGui::DragFloat2("Offset", emitter->data.offset.data);
    ImGui::DragInt("Burst", &emitter->data.burst);
    ImGui::DragFloat("Interval", &emitter->data.interval);
    ImGui_Separator();

    ImGui::DragFloat2("burst_rotation_delta", (float*)&emitter->data.burst_rotation_delta);
    ImGui::DragFloat2("speed", (float*)&emitter->data.speed);
    ImGui::DragFloat2("acc", (float*)&emitter->data.acc);
    ImGui::DragFloat2("dir", (float*)&emitter->data.dir);
}

inline void guiParticleLayer2D(void* comp) {
    particle_layer2d_t* layer = (particle_layer2d_t*)comp;
    ImGui::Checkbox("Keep Alive", &layer->keep_alive);
    ImGui::LabelText("Num Particles", "%u", arr_size(layer->particles));
}

static void gui_inspector(entity_t e) {
    ImGui_PushID((uintptr_t)e.id);
    ImGui::LabelText("ID", "#%02X_%04X", e.gen, e.idx);

    const node_t* node = get_node(e);
    if (node) {
        ImGui::LabelText("Tag", "%s [0x%08X]", hsp_get(node->tag), node->tag);

        bool visible = is_visible(e);
        if (ImGui::Checkbox("Visible", &visible)) {
            set_visible(e, visible);
        }

        bool touchable = is_touchable(e);
        if (ImGui::Checkbox("Touchable", &touchable)) {
            set_touchable(e, touchable);
        }
    }

    guiComponentPanel("Transform2D", get_transform2d(e), guiTransform2D);
    guiComponentPanel("Viewport", get_viewport(e), guiViewport);
    guiComponentPanel("Layout", get_layout_rect(e), guiLayout);
    guiComponentPanel("Camera2D", get_camera2d(e), guiCamera2D);
    guiComponentPanel("Bounds2D", get_bounds2d(e), guiBounds2D);

    if (Transform3D.index) {
        guiComponentPanel("Transform 3D", get_transform3d(e), guiTransform3D);
        guiComponentPanel("Camera 3D", get_camera3d(e), guiCamera3D);
        guiComponentPanel("Light 3D", get_light3d(e), guiLight3D);
        guiComponentPanel("Mesh Renderer", get_mesh_renderer(e), guiMeshRenderer);
    }

    guiComponentPanel("Interactive", get_interactive(e), gui_interactive);

    guiComponentPanel("Node Events", get_node_events(e), gui_node_events);

    // particles
    guiComponentPanel("ParticleEmitter2D", get_particle_emitter2d(e), guiParticleEmitter2D);
    guiComponentPanel("ParticleLayer2D", get_particle_layer2d(e), guiParticleLayer2D);
    guiComponentPanel("ParticleRenderer2D", get_particle_renderer2d(e), editParticleRenderer2D);

    // display2d
    guiComponentPanel("Quad2D", get_quad2d(e), editDisplayRectangle);
    guiComponentPanel("Sprite2D", get_sprite2d(e), editDisplaySprite);
    guiComponentPanel("NinePatch2D", get_ninepatch2d(e), editDisplayNinePatch);
    guiComponentPanel("Text2D", get_text2d(e), editDisplayText);
    guiComponentPanel("Arc2D", get_arc2d(e), editDisplayArc);

    guiComponentPanel("Movie Clip", get_movieclip(e), guiMovieClip);

    ImGui_PopID();
}
