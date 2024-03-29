#pragma once

#include "SceneWindow.hpp"
#include <ImGuizmo/ImGuizmo.h>

SceneWindow editor_scene_window;

// matrix 2d utility
mat3x2_t matrix3Dto2D(const mat4_t m) {
    mat3x2_t result;
    result.a = m.m00;
    result.b = m.m01;
    result.c = m.m10;
    result.d = m.m11;
    result.tx = m.m30;
    result.ty = m.m31;
    return result;
}

mat4_t matrix2Dto3D(const mat3x2_t m) {
    mat4_t result = mat4_identity();
    result.m00 = m.a;
    result.m01 = m.b;
    result.m10 = m.c;
    result.m11 = m.d;
    result.m30 = m.tx;
    result.m31 = m.ty;
    return result;
}

vec2_t SceneView2D::getMouseWorldPos(vec2_t pos) const {
    const bool ok = vec2_transform_inverse(pos, matrix, &pos);
    EK_ASSERT(ok);
    return pos;
}

void SceneView2D::reset() {
    matrix = mat3x2_identity();
    position = {};
    scale = 1.0f;
    translation = {};
}

void SceneView2D::manipulateView(vec2_t mouseWorldPosition, const rect_t viewport) {
    if (ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonRight)) {
        const auto delta = ImGui::GetMouseDragDelta(ImGuiPopupFlags_MouseButtonRight);
        translation.x = delta.x;
        translation.y = delta.y;
    } else {
        position += translation;
        translation = {};
    }

    const auto wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f) {
        float newScale = fmax(scaleMin, scale + wheel);
        float deltaScale = newScale - scale;
        scale = newScale;
        position -= mouseWorldPosition * deltaScale;
    }

    const vec2_t ps = position + translation;
    const vec2_t sc = vec2(scale, scale);
    const vec2_t rt = vec2(0, 0);
    matrix.pos = ps;
    matrix.rot = mat2_scale_skew(sc, rt);
    projectionMatrix = mat4_orthographic_2d(0, 0, viewport.w, viewport.h, -1000.0f, 1000.0f);

    viewMatrix3D = mat4_2d_transform(ps, sc, rt);
}

void SceneView3D::reset() {
    position = {};
    translation = {};
}

vec2_t SceneView3D::getMouseWorldPos(vec2_t viewportMousePosition) const {
    // TODO:
    return viewportMousePosition;
}

vec2_t SceneView::getMouseWorldPos() const {
    const auto mousePos = ImGui::GetMousePos();
    const vec2_t pos = vec2(mousePos.x, mousePos.y) - rect.position;

    if (mode2D) return view2.getMouseWorldPos(pos);
    return view3.getMouseWorldPos(pos);
}

void SceneView::reset() {
    if (mode2D) view2.reset();
    else view3.reset();
}

void SceneView::manipulateView() {
    if (mode2D) view2.manipulateView(getMouseWorldPos(), rect);
    else {
        // TODO:
    }
}

void draw_scene_window(void) {
    editor_scene_window.drawToolbar();

    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const ImVec2 displaySize = ImGui::GetContentRegionAvail();
    const bool mouseInView = ImGui::IsWindowHovered() &&
                             ImGui::IsMouseHoveringRect(displayPos,
                                                        {displayPos.x + displaySize.x,
                                                         displayPos.y + displaySize.y}
                             );

    const bool canManipulateView = mouseInView && !ImGuizmo::IsUsing();
    const bool canSelectObjects = mouseInView && !(ImGuizmo::IsUsing() || ImGuizmo::IsOver());

    editor_scene_window.view.rect = rect(displayPos.x, displayPos.y, displaySize.x, displaySize.y);
    if (canManipulateView) {
        editor_scene_window.view.manipulateView();
    }

    // update size
    const float k = editor_scene_window.display.info.dpi_scale;
    editor_scene_window.display.info.dest_viewport = rect(
            k * displayPos.x, k * displayPos.y,
            k * displaySize.x, k * displaySize.y
    );
    editor_scene_window.display.info.window.x = displaySize.x;
    editor_scene_window.display.info.window.y = displaySize.y;
    editor_scene_window.display.info.size.x = k * displaySize.x;
    editor_scene_window.display.info.size.y = k * displaySize.y;
    game_display_update(&editor_scene_window.display);

    // add pass to render imgui
    if (editor_scene_window.display.color.id && displaySize.x > 0 && displaySize.y > 0) {
        auto texId = (void*) static_cast<uintptr_t>(editor_scene_window.display.color.id);

        const sg_image_desc info = sg_query_image_desc(editor_scene_window.display.color);
        const float texCoordX1 = editor_scene_window.display.info.size.x / (float) info.width;
        const float texCoordY1 = editor_scene_window.display.info.size.y / (float) info.height;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage(texId, {displayPos.x, displayPos.y},
                           {displayPos.x + editor_scene_window.display.info.size.x,
                            displayPos.y + editor_scene_window.display.info.size.y},
                           {0, 0},
                           {texCoordX1, texCoordY1});
    }

    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(displayPos.x, displayPos.y, displaySize.x, displaySize.y);

    if (!editor_scene_window.view.mode2D) {
        ImGuizmo::ViewManipulate(editor_scene_window.view.view3.viewMatrix.data, 100.0f,
                                 ImVec2(displayPos.x + displaySize.x - 128.0f, displayPos.y), ImVec2(100.0f, 100.0f),
                                 0x10101010);
    }

    bool manipulationToolSelected = editor_scene_window.currentTool >= 2 && editor_scene_window.currentTool <= 6;
    if (manipulationToolSelected) {
        if (editor_scene_window.view.mode2D) {
            editor_scene_window.manipulateObject2D();
        } else {
            editor_scene_window.manipulateObject3D();
        }
    }

    if (is_entity(editor_scene_window.root) && canSelectObjects) {
        const auto wp = editor_scene_window.view.getMouseWorldPos();
        entity_t target = editor_scene_window.hitTest(editor_scene_window.root, wp);
        editor_scene_window.hoverTarget = target;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            editor_hierarchy_window.select(target);
            editor_hierarchy_window.focus(target);
        }
    }
}

SceneWindow::SceneWindow() {
    display.simulated = true;
    display.info.size = {120, 120};
    display.info.window = {120, 120};
    display.info.dpi_scale = 1.0f;

    view.view3.projectionMatrix = mat4_perspective_rh(to_radians(45.0f), 4.0f / 3.0f, 10.0f, 1000.0f);
    view.view3.viewMatrix = mat4_look_at_rh(vec3(0, 0, 100), {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
}

SceneWindow::~SceneWindow() = default;

void drawBox2(rect_t rc, mat3x2_t m, color_t color1, color_t color2,
              bool cross = true, color_t fillColor = COLOR_ZERO) {

    canvas_set_empty_image();
    if (fillColor.value != 0) {
        canvas_save_matrix();
        canvas.matrix[0] = m;
        canvas_fill_rect(rc, fillColor);
        canvas_restore_matrix();
    }
    auto bb = aabb2_from_rect(rc);
    auto v1 = vec2_transform(bb.min, m);
    auto v2 = vec2_transform(vec2(bb.x1, bb.y0), m);
    auto v3 = vec2_transform(bb.max, m);
    auto v4 = vec2_transform(vec2(bb.x0, bb.y1), m);
    canvas_line_ex(v1, v2, color1, color2, 1, 1);
    canvas_line_ex(v2, v3, color1, color2, 1, 1);
    canvas_line_ex(v3, v4, color1, color2, 1, 1);
    canvas_line_ex(v4, v1, color1, color2, 1, 1);
    if (cross) {
        canvas_line_ex(v1, v3, color1, color2, 1, 1);
        canvas_line_ex(v2, v4, color1, color2, 1, 1);
    }
}

void SceneWindow::drawSceneNode(entity_t e) {
    if (!is_visible(e)) {
        return;
    }

    auto* disp = get_display2d(e);
    if (disp) {
        auto* transform = get_world_transform2d(e);
        if (transform) {
            canvas.matrix[0] = transform->matrix;
            canvas.color[0] = transform->color;
        } else {
            canvas.matrix[0] = mat3x2_identity();
            canvas.color[0] = color2_identity();
        }
        if (disp->draw) {
            disp->draw(e);
        }
    }
    foreach_child(e, drawSceneNode);
}

void SceneWindow::drawSceneNodeBounds(entity_t e) {
    if (!is_visible(e)) {
        return;
    }

    auto* disp = get_display2d(e);
    if (disp) {
        canvas.matrix[0] = mat3x2_identity();
        canvas.color[0] = color2_identity();

        mat3x2_t m = view.view2.matrix;
        auto* transform = get_world_transform2d(e);
        if (transform) {
            m = mat3x2_mul(view.view2.matrix, transform->matrix);
        }
        rect_t b = disp->get_bounds ? disp->get_bounds(e) : rect_wh(0,0);
        if (editor_hierarchy_window.isSelectedInHierarchy(e)) {
            drawBox2(b, m, COLOR_WHITE, COLOR_BLACK, true, ARGB(0x77FFFFFF));
        }
        if (hoverTarget.id == e.id) {
            drawBox2(b, m, ARGB(0x77FFFFFF), ARGB(0x77000000), false);
        }
    }
    auto it = get_first_child(e);
    while (it.id) {
        drawSceneNodeBounds(it);
        it = get_next_child(it);
    }
}

void SceneWindow::pre_render() {
    if (display.color.id == SG_INVALID_ID) {
        return;
    }
    sg_pass_action passAction{};
    passAction.colors[0].action = SG_ACTION_CLEAR;
    passAction.colors[0].value = {0.5f, 0.5f, 0.5f, 1.0f};
    passAction.depth.action = SG_ACTION_CLEAR;
    passAction.depth.value = 1.0f;
    if (game_display_begin(&display, &passAction, "scene-view")) {
        drawScene();
        game_display_end(&display);
    }
}

void SceneWindow::drawScene() {
    if (!is_entity(root)) {
        root = game_app_state.root;
    }

    canvas_begin_ex({0, 0, display.info.size.x, display.info.size.y}, view.view2.matrix, {0}, {0});
    if (!view.mode2D) {
        canvas.mvp = mat4_mul(view.view3.projectionMatrix, view.view3.viewMatrix);
    }
    drawSceneNode(root);
    canvas_end();

    canvas_begin(display.info.size.x, display.info.size.y);
    if (!view.mode2D) {
        canvas.mvp = mat4_mul(view.view3.projectionMatrix, view.view3.viewMatrix);
    }
    drawSceneNodeBounds(root);
    canvas_end();
}

entity_t SceneWindow::hitTest(entity_t e, vec2_t worldPos) {
    const node_t* node = get_node(e);
    if (node->flags & (NODE_HIDDEN | NODE_UNTOUCHABLE)) {
        return NULL_ENTITY;
    }
    entity_t it = get_last_child(e);
    while (it.id) {
        entity_t t = hitTest(it, worldPos);
        if (t.id) {
            return t;
        }
        it = get_prev_child(it);
    }
    auto* disp = get_display2d(e);
    if (disp && disp->get_bounds) {
        auto* wt = get_world_transform2d(e);
        if (wt) {
            vec2_t lp;
            if (vec2_transform_inverse(worldPos, wt->matrix, &lp) &&
                rect_contains(disp->get_bounds(e), lp)) {
                return e;
            }
        }
    }
    return NULL_ENTITY;
}

void SceneWindow::drawToolbar() {
    if (ImGui::BeginPopupContextItem("###scene_camera_menu")) {
        if (ImGui::MenuItem("Reset")) {
            view.reset();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button(ICON_FA_CAMERA)) {
        ImGui::OpenPopup("###scene_camera_menu");
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton("2D", view.mode2D, "Toggle 2D/3D")) {
        view.mode2D = !view.mode2D;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_MOUSE_POINTER, currentTool == 0, "Select")) {
        currentTool = 0;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_HAND_PAPER, currentTool == 1, "Pan")) {
        currentTool = 1;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_ARROWS_ALT, currentTool == 2, "Translate")) {
        currentTool = 2;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_CIRCLE, currentTool == 3, "Rotate")) {
        currentTool = 3;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_EXPAND, currentTool == 4, "Scale")) {
        currentTool = 4;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_ADJUST, currentTool == 5, "Manipulate")) {
        currentTool = 5;
    }
    ImGui::SameLine();
    if (ImGui_ToolbarButton(ICON_FA_VECTOR_SQUARE, currentTool == 6, "Bounds")) {
        currentTool = 6;
    }

    if (currentTool != 4) {
        ImGui::SameLine();
        if (ImGui_ToolbarButton(ICON_FA_OBJECT_GROUP, localGlobal == 0, "Object Space")) {
            localGlobal = 0;
        }
        ImGui::SameLine();
        if (ImGui_ToolbarButton(ICON_FA_OBJECT_UNGROUP, localGlobal == 1, "World Space")) {
            localGlobal = 1;
        }
    }
}

void SceneWindow::manipulateObject2D() {
    auto& selection = editor_hierarchy_window.selection;
    if (selection.size() > 0 && is_entity(entity_id(selection[0]))) {
        entity_t sel = entity_id(selection[0]);
        auto worldMatrix2D = get_world_transform2d(sel)->matrix;
        mat4_t worldMatrix3D = matrix2Dto3D(worldMatrix2D);
        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::BOUNDS;
        if (currentTool == 2) {
            op = ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
        } else if (currentTool == 3) {
            op = ImGuizmo::OPERATION::ROTATE_Z;
        } else if (currentTool == 4) {
            op = ImGuizmo::OPERATION::SCALE;
        } else if (currentTool == 5) {
            op = ImGuizmo::OPERATION::SCALE | ImGuizmo::OPERATION::ROTATE_Z | ImGuizmo::OPERATION::TRANSLATE_X |
                 ImGuizmo::OPERATION::TRANSLATE_Y;
        }
        ImGuizmo::MODE mode = (localGlobal == 0 && currentTool != 4) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        ImGuizmo::Manipulate(view.view2.viewMatrix3D.data, view.view2.projectionMatrix.data,
                             op, mode,
                             worldMatrix3D.data,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr);

        transform2d_t* localTransform = get_transform2d(sel);
        const auto parentWorldMatrix2D = get_world_transform2d(get_parent(sel))->matrix;
        const auto parentWorldMatrix3D = matrix2Dto3D(parentWorldMatrix2D);
        const auto inverseParentWorldMatrix3D = mat4_inverse(parentWorldMatrix3D);
        const auto newLocalMatrix3D = mat4_mul(inverseParentWorldMatrix3D, worldMatrix3D);
        transform2d_set_matrix(localTransform, matrix3Dto2D(newLocalMatrix3D));
    }
}

void SceneWindow::manipulateObject3D() {
    auto& selection = editor_hierarchy_window.selection;
    if (selection.size() > 0 && is_entity(entity_id(selection[0]))) {
        entity_t sel = entity_id(selection[0]);
        auto worldMatrix2D = get_world_transform2d(sel)->matrix;
        mat4_t worldMatrix3D = matrix2Dto3D(worldMatrix2D);
        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::BOUNDS;
        if (currentTool == 2) {
            op = ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
        } else if (currentTool == 3) {
            op = ImGuizmo::OPERATION::ROTATE_Z;
        } else if (currentTool == 4) {
            op = ImGuizmo::OPERATION::SCALE;
        } else if (currentTool == 5) {
            op = ImGuizmo::OPERATION::SCALE | ImGuizmo::OPERATION::ROTATE_Z | ImGuizmo::OPERATION::TRANSLATE_X |
                 ImGuizmo::OPERATION::TRANSLATE_Y;
        }
        ImGuizmo::MODE mode = (localGlobal == 0 && currentTool != 4) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        ImGuizmo::Manipulate(view.view3.viewMatrix.data, view.view3.projectionMatrix.data,
                             op, mode,
                             worldMatrix3D.data,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr);

//        auto& localTransform = get_transform2d(sel);
//        const auto parentWorldMatrix2D = get_node(sel).get_world_transform2d(parent).matrix;
//        const auto parentWorldMatrix3D = matrix2Dto3D(parentWorldMatrix2D);
//        const auto inverseParentWorldMatrix3D = inverse(parentWorldMatrix3D);
//        const auto newLocalMatrix3D = inverseParentWorldMatrix3D * worldMatrix3D;
//        localTransform.setMatrix(matrix3Dto2D(newLocalMatrix3D));
    }
}
