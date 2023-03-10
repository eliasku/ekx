#pragma once

#include "Widgets.hpp"
#include <ecx/ecx.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/3d/StaticMesh.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/Viewport.hpp>

namespace ek {

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
        ImGui::TextDisabled("%s: null", label);
    } else if (!is_entity(e)) {
        ImGui::TextColored({1, 0, 0, 1}, "%s: invalid", label);
    } else {
        auto tag = get_tag(e);
        ImGui::LabelText(label, "%s [%08X]", hsp_get(tag), tag);
    }
}

template<typename C, typename Func>
inline void guiComponentPanel(const char* name, C& data, Func fn) {
    if (ImGui::CollapsingHeader(name)) {
        ImGui::PushID(&data);
        ImGui::Indent();
        fn(data);
        ImGui::Unindent();
        ImGui::PopID();
    }
}

template<typename C, typename Func>
inline void guiComponentPanel(ecs::Entity entity, const char* name, Func fn) {
    if (ecs::has<C>(entity)) {
        auto& data = ecs::get<C>(entity);
        guiComponentPanel(name, data, fn);
    }
}

inline void guiMovieClip(MovieClip& mc) {
    const auto* data = mc.data;
    if (data) {
        ImGui::LabelText("Total Frames", "%u", data->frames);
        ImGui::LabelText("Default FPS", "%f", data->fps);

        ImGui::DragFloat("Time", &mc.time, 1.0f, 0.0f, (float) data->frames);
        ImGui::DragFloat("FPS", &mc.fps, 1.0f, 0.0f, 100.0f);
        ImGui::Checkbox("Playing", &mc.playing);
    }
}

inline void guiTransform2D(Transform2D& transform) {
    auto pos = transform.getPosition();
    auto scale = transform.getScale();
    auto skew = transform.getSkew();
    if (ImGui::DragFloat2("Position", pos.data, 1.0f, 0.0f, 0.0f, "%.1f")) {
        transform.set_position(pos);
    }
    if (ImGui::DragFloat2("Scale", scale.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.set_scale(scale);
    }
    if (ImGui::DragFloat2("Skew", skew.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.setSkew(skew);
    }
//    ImGui::DragFloat2("Origin", transform.origin.data(), 0.1f, 0.0f, 0.0f, "%.2f");
//    ImGui::DragFloat2("Pivot", transform.pivot.data(), 0.1f, 0.0f, 0.0f, "%.2f");

    auto color = vec4_color(transform.color.scale);
    if (ImGui::ColorEdit4("Color Scale", color.data)) {
        transform.color.scale = color_vec4(color);
    }

    color = vec4_color(transform.color.offset);
    if (ImGui::ColorEdit4("Color Offset", color.data)) {
        transform.color.offset = color_vec4(color);
    }
}

inline void guiViewport(Viewport& vp) {
    ImGui::EditRect("Viewport", vp.options.viewport.data);
    ImGui::DragFloat2("Alignment", vp.options.alignment.data);
    ImGui::DragFloat2("Base Resolution", vp.options.baseResolution.data);
    ImGui::DragFloat2("Pixel Ratio", vp.options.pixelRatio.data);
    ImGui::DragFloat2("Safe Area Fit", vp.options.safeAreaFit.data);
    ImGui::Checkbox("Scale To Resolution", &vp.options.scaleToResolution);
    if (ImGui::CollapsingHeader("Debug Output")) {
        ImGui::Indent();
        ImGui::EditRect("Screen Rect", vp.output.screenRect.data);
        ImGui::EditRect("Full Rect", vp.output.fullRect.data);
        ImGui::EditRect("Safe Rect", vp.output.safeRect.data);
        ImGui::InputFloat2("Offset", vp.output.offset.data);
        ImGui::InputFloat("Scale", &vp.output.scale);
        ImGui::Unindent();
    }
}

inline void guiCamera2D(Camera2D& camera) {
    ImGui::Checkbox("Enabled", &camera.enabled);
    ImGui::Checkbox("interactive", &camera.interactive);
    ImGui::Checkbox("occlusionEnabled", &camera.occlusionEnabled);
    ImGui::DragInt("Order", &camera.order);
    guiEntityRef("Root Entity", camera.root);
    ImGui::DragFloat("Content Scale", &camera.contentScale);
    ImGui::Checkbox("Clear Color", &camera.clearColorEnabled);
    ImGui::ColorEdit4("Clear Color", camera.clearColor.data);
    ImGui::ColorEdit4("Clear Color+", camera.clearColor2.data);
    //ImGui::EditRect("viewport", camera.viewport.data());
    ImGui::DragFloat2("relativeOrigin", camera.relativeOrigin.data);

    ImGui::Separator();
    ImGui::Checkbox("Draw Occlusion", &camera.debugOcclusion);
    ImGui::Checkbox("Draw Bounds", &camera.debugVisibleBounds);
    ImGui::Checkbox("Draw Hit Target", &camera.debugGizmoHitTarget);

    ImGui::Checkbox("Draw Pointer", &camera.debugGizmoPointer);
    ImGui::Checkbox("Draw Camera Gizmo", &camera.debugGizmoSelf);
    ImGui::DragFloat("Debug Scale", &camera.debugDrawScale);
}

inline void guiTransform3D(Transform3D& transform) {
    ImGui::DragFloat3("Position", transform.position.data, 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat3("Scale", transform.scale.data, 0.1f, 0.0f, 0.0f, "%.2f");
    vec3_t euler_angles = scale_vec3(transform.rotation, 180.0f / MATH_PI);
    if (ImGui::DragFloat3("Rotation", euler_angles.data, 0.1f, 0.0f, 0.0f, "%.2f")) {
        transform.rotation = scale_vec3(euler_angles, MATH_PI / 180.0f);
    }
}

inline void guiCamera3D(Camera3D& camera) {
    ImGui::DragFloatRange2("Clip Plane", &camera.zNear, &camera.zFar, 1.0f, 0.0f, 0.0f, "%.1f");
    float fov_degree = to_degrees(camera.fov);
    if (ImGui::DragFloat("FOV", &fov_degree, 1.0f, 0.0f, 0.0f, "%.1f")) {
        camera.fov = to_radians(fov_degree);
    }

    ImGui::Checkbox("Orthogonal", &camera.orthogonal);
    ImGui::DragFloat("Ortho Size", &camera.orthogonalSize, 1.0f, 0.0f, 0.0f, "%.1f");

    ImGui::Checkbox("Clear Color Enabled", &camera.clearColorEnabled);
    ImGui::ColorEdit4("Clear Color", camera.clearColor.data);
    ImGui::Checkbox("Clear Depth Enabled", &camera.clearDepthEnabled);
    ImGui::DragFloat("Clear Depth", &camera.clearDepth, 1.0f, 0.0f, 0.0f, "%.1f");
}

inline void guiMeshRenderer(MeshRenderer& renderer) {
    ImGui::Checkbox("Cast Shadows", &renderer.castShadows);
    ImGui::Checkbox("Receive Shadows", &renderer.receiveShadows);
}

inline void guiLight3D(Light3D& light) {
    if (light.type == Light3DType::Directional) {
        ImGui::Text("Directional Light");
    } else if (light.type == Light3DType::Point) {
        ImGui::Text("Point Light");
    } else if (light.type == Light3DType::Spot) {
        ImGui::Text("Spot Light");
    }
    ImGui::ColorEdit3("Ambient", light.ambient.data);
    ImGui::ColorEdit3("Diffuse", light.diffuse.data);
    ImGui::ColorEdit3("Specular", light.specular.data);

    ImGui::DragFloat("Radius", &light.radius, 1.0f, 0.0f, 0.0f, "%.1f");
    ImGui::DragFloat("Falloff", &light.falloff, 0.1f, 0.0f, 0.0f, "%.1f");
}

inline void guiBounds2D(Bounds2D& bounds) {
    ImGui::EditRect("Rect", bounds.rect.data);
    ImGui::CheckboxFlags("Hit Area", &bounds.flags, BOUNDS_2D_HIT_AREA);
    ImGui::CheckboxFlags("Scissors", &bounds.flags, BOUNDS_2D_SCISSORS);
    ImGui::CheckboxFlags("Cull Box", &bounds.flags, BOUNDS_2D_CULL);
}

inline void guiInteractive(Interactive& inter) {
    ImGui::Checkbox("pushed", &inter.pushed);
    ImGui::Checkbox("over", &inter.over);
    ImGui::Checkbox("bubble", &inter.bubble);
    ImGui::LabelText("cursor", inter.cursor == EK_MOUSE_CURSOR_BUTTON ? "button" : "?");
}

inline void spriteRefInfo(R(sprite_t) ref) {
    sprite_t spr = REF_RESOLVE(res_sprite, ref);
    ImGui::LabelText("Image", "ref: %u", spr.image_id);
    ImGui::LabelText("Loaded", "%u", !!(spr.state & SPRITE_LOADED));
    ImGui::LabelText("Rotated", "%u", !!(spr.state & SPRITE_ROTATED));
    ImGui::TextDisabled("TODO: select sprite res %u", ref);
}

inline void editDisplaySprite(Sprite2D& sprite) {
    spriteRefInfo(sprite.src);
    ImGui::Checkbox("Hit Pixels", &sprite.hit_pixels);
}

inline void editDisplayNinePatch(NinePatch2D& ninePatch) {
    spriteRefInfo(ninePatch.src);
    // TODO: scale, size
    //ImGui::Checkbox("Scale Grid", &ninePatch.scale_grid_mode);
    ImGui::Checkbox("Hit Pixels", &ninePatch.hit_pixels);
}

inline void editDisplayRectangle(Quad2D& quad) {
    ImGui::EditRect("Bounds", quad.rect.data);
    ImGui::Color32Edit("Color LT", &quad.colors[0]);
    ImGui::Color32Edit("Color RT", &quad.colors[1]);
    ImGui::Color32Edit("Color RB", &quad.colors[2]);
    ImGui::Color32Edit("Color LB", &quad.colors[3]);
}

inline void editDisplayArc(Arc2D& arc) {
    spriteRefInfo(arc.sprite);
    ImGui::DragFloat("Angle", &arc.angle);
    ImGui::DragFloat("Radius", &arc.radius);
    ImGui::DragFloat("Line Width", &arc.line_width);
    ImGui::DragInt("Segments", &arc.segments);
    ImGui::Color32Edit("Color Inner", &arc.color_inner);
    ImGui::Color32Edit("Color Outer", &arc.color_outer);
}

inline void editParticleRenderer2D(ParticleRenderer2D& p) {
    guiEntityRef("Target", p.target);
}

inline void guiTextFormat(TextFormat& format) {
    select_ref_asset("Font", &res_font.rr, &format.font);
    ImGui::DragFloat("Size", &format.size, 1, 8, 128, "%f");
    ImGui::DragFloat("Leading", &format.leading, 1, 0, 128, "%f");
    ImGui::DragFloat("Spacing", &format.letterSpacing, 1, -128, 128, "%f");
    ImGui::DragFloat2("Alignment", format.alignment.data, 0.5f, 0.0f, 1.0f);
    ImGui::Checkbox("Kerning", &format.kerning);
    ImGui::Checkbox("Underline", &format.underline);
    ImGui::Checkbox("Strikethrough", &format.strikethrough);

    ImGui::SliderInt("Layers", &format.layersCount, 0, TextFormat::LayersMax);
    if (ImGui::Checkbox("Show all glyph bounds", &format.layers[0].showGlyphBounds)) {
        bool sgb = format.layers[0].showGlyphBounds;
        for (int i = 0; i < TextFormat::LayersMax; ++i) {
            format.layers[i].showGlyphBounds = sgb;
        }
    }

    ImGui::Indent();
    for (int i = 0; i < format.layersCount; ++i) {
        auto& layer = format.layers[i];
        guiTextLayerEffect(layer);
    }
    ImGui::Unindent();
}

inline void editDisplayText(Text2D& tf) {
    // TODO:
    //ImGui::InputTextMultiline("Text", &tf.str_buf);
    ImGui::LabelText("Text", "%s", text2d__c_str(&tf));
    ImGui::EditRect("Bounds", tf.rect.data);
    ImGui::Color32Edit("Border Color", &tf.borderColor);
    ImGui::Color32Edit("Fill Color", &tf.fillColor);
    ImGui::Checkbox("Hit Full Bounds", &tf.hitFullBounds);
    ImGui::Checkbox("Show Text Bounds", &tf.showTextBounds);
    guiTextFormat(tf.format);
}

inline void guiLayout(LayoutRect& layout) {
    ImGui::Checkbox("Fill X", &layout.fill_x);
    ImGui::Checkbox("Fill Y", &layout.fill_y);
    ImGui::Checkbox("Align X", &layout.align_x);
    ImGui::Checkbox("Align Y", &layout.align_y);
    ImGui::Checkbox("Use Safe Insets", &layout.doSafeInsets);
    ImGui::EditRect("Extra Fill", layout.fill_extra.data);
    ImGui::DragFloat2("Align X (rel, abs)", layout.x.data);
    ImGui::DragFloat2("Align Y (rel, abs)", layout.y.data);

    ImGui::EditRect("Rect", layout.rect.data);
    ImGui::EditRect("Safe Rect", layout.safeRect.data);
}

inline void guiParticleEmitter2D(ParticleEmitter2D& emitter) {
    ImGui::Checkbox("Enabled", &emitter.enabled);
    ImGui::Text("_Time: %f", emitter.time);
    guiEntityRef("Layer", emitter.layer);
    ImGui::LabelText("Particle", "SID: %u ( %s )", emitter.particle, hsp_get(res_particle.names[emitter.particle]));
    ImGui::DragFloat2("Offset", emitter.position.data);
    ImGui::Separator();

    // data
    ImGui::EditRect("Rect", emitter.data.rect.data);
    ImGui::DragFloat2("Offset", emitter.data.offset.data);
    ImGui::DragInt("Burst", &emitter.data.burst);
    ImGui::DragFloat("Interval", &emitter.data.interval);
    ImGui::Separator();

    ImGui::DragFloat2("burst_rotation_delta", emitter.data.burst_rotation_delta.data());
    ImGui::DragFloat2("speed", emitter.data.speed.data());
    ImGui::DragFloat2("acc", emitter.data.acc.data());
    ImGui::DragFloat2("dir", emitter.data.dir.data());
}

inline void guiParticleLayer2D(ParticleLayer2D& layer) {
    ImGui::Checkbox("Keep Alive", &layer.keepAlive);
    ImGui::LabelText("Num Particles", "%u", layer.particles.size());
}

void InspectorWindow::gui_inspector(ecs::Entity e) {
    ImGui::PushID(e.id);
    ImGui::LabelText("ID", "#%02X_%04X", e.gen, e.idx);
    if (ecs::has<Node>(e)) {
        auto& node = ecs::get<Node>(e);
        ImGui::LabelText("Tag", "%s [0x%08X]", hsp_get(node.tag), node.tag);

        bool visible = is_visible(e);
        if(ImGui::Checkbox("Visible", &visible)) {
            set_visible(e, visible);
        }

        bool touchable = is_touchable(e);
        if(ImGui::Checkbox("Touchable", &touchable)) {
            set_touchable(e, touchable);
        }
    }

    guiComponentPanel<Transform2D>(e, "Transform2D", guiTransform2D);
    guiComponentPanel<Viewport>(e, "Viewport", guiViewport);
    guiComponentPanel<LayoutRect>(e, "Layout", guiLayout);
    guiComponentPanel<Camera2D>(e, "Camera2D", guiCamera2D);
    guiComponentPanel<Bounds2D>(e, "Bounds2D", guiBounds2D);

    if (ecs::has_type<Transform3D>()) {
        guiComponentPanel<Transform3D>(e, "Transform 3D", guiTransform3D);
        guiComponentPanel<Camera3D>(e, "Camera 3D", guiCamera3D);
        guiComponentPanel<Light3D>(e, "Light 3D", guiLight3D);
        guiComponentPanel<MeshRenderer>(e, "Mesh Renderer", guiMeshRenderer);
    }

    guiComponentPanel<Interactive>(e, "Interactive", guiInteractive);
    guiComponentPanel<NodeEventHandler>(e, "Event Handler", [](auto& c) {});

    // particles
    guiComponentPanel<ParticleEmitter2D>(e, "ParticleEmitter2D", guiParticleEmitter2D);
    guiComponentPanel<ParticleLayer2D>(e, "ParticleLayer2D", guiParticleLayer2D);

    // display2d
    guiComponentPanel<Sprite2D>(e, "Sprite2D", editDisplaySprite);
    guiComponentPanel<NinePatch2D>(e, "NinePatch2D", editDisplayNinePatch);
    guiComponentPanel<Quad2D>(e, "Quad2D", editDisplayRectangle);
    guiComponentPanel<Text2D>(e, "Text2D", editDisplayText);
    guiComponentPanel<Arc2D>(e, "Arc2D", editDisplayArc);
    guiComponentPanel<ParticleRenderer2D>(e, "ParticleRenderer2D", editParticleRenderer2D);

    guiComponentPanel<MovieClip>(e, "Movie Clip", guiMovieClip);

    ImGui::PopID();
}

void InspectorWindow::onDraw() {
    if (!list.empty()) {
        if (list.size() > 1) {
            ImGui::LabelText("Multiple Selection", "%u", list.size());
        } else {
            gui_inspector(list[0]);
        }
    }
}

}