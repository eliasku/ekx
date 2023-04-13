#pragma once


#include "HierarchyWindow.hpp"
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/text2d.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/viewport.h>
#include <ek/scenex/2d/camera2d.h>
#include <ek/scenex/3d/scene3d.h>

namespace ek {

entity_t HierarchyWindow::getSiblingNext(entity_t e) {
    const auto* node = Node_get(e);
    return node ? node->sibling_next : NULL_ENTITY;
}

const char* HierarchyWindow::getEntityIcon(entity_t e) {
    if (get_camera2d(e)) return ICON_FA_VIDEO;
    if (get_viewport(e)) return ICON_FA_TV;
    if (get_bounds2d(e)) return ICON_FA_EXPAND;
    if (get_button(e)) return ICON_FA_HAND_POINTER;
    if (interactive_get(e)) return ICON_FA_FINGERPRINT;
    if (get_movieclip(e)) return ICON_FA_FILM;
    if (get_sprite2d(e)) return ICON_FA_IMAGE;
    if (get_ninepatch2d(e)) return ICON_FA_COMPRESS;
    if (get_quad2d(e)) return ICON_FA_VECTOR_SQUARE;
    if (get_text2d(e)) return ICON_FA_FONT;
    if (get_arc2d(e)) return ICON_FA_CIRCLE_NOTCH;
    if (get_display2d(e)) return ICON_FA_PAINT_BRUSH;
    if (get_transform2d(e)) return ICON_FA_DICE_D6;
    if (Node_get(e)) return ICON_FA_BOX;

    if (Transform3D.index && get_transform3d(e)) return ICON_FA_DICE_D20;

    return ICON_FA_BORDER_STYLE;
}

void getEntityTitle(entity_t e, char buffer[64]) {
    auto tag = get_tag(e);
    if (tag) {
        const char* str = hsp_get(tag);
        if (*str) {
            strcpy(buffer, str);
        } else {
            ek_snprintf(buffer, 64, "[0x%08X]", tag);
        }
    } else {
        strcpy(buffer, "Entity");
    }
}

bool HierarchyWindow::isSelectedInHierarchy(entity_t ref) {
    const entity_id_t* it = selection.find(ref.id);
    return it != nullptr;
}

bool HierarchyWindow::hasChildren(entity_t e) {
    node_t* node = Node_get(e);
    if (node) {
        auto first = node->child_first;
        return first.id && is_entity(first);
    }
    return false;
}

bool HierarchyWindow::hoverIconButton(const char* str_id, const char* icon) {
    (void)str_id;
    ImGui::TextUnformatted(icon);
    return ImGui::IsItemClicked();
}

void HierarchyWindow::drawVisibleTouchControls(node_t* node, bool parentedVisible, bool parentedTouchable) {
    if (!node) {
        return;
    }
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(10);
    bool is_visible = !(node->flags & NODE_HIDDEN);
    bool is_touchable = !(node->flags & NODE_UNTOUCHABLE);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (parentedVisible || !is_visible) ? 1.0f : 0.5f);
    {
        const char* icon = is_visible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
        if (hoverIconButton("visible", icon)) {
            node->flags ^= NODE_HIDDEN;
        }
    }
    ImGui::PopStyleVar();

    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(30);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (parentedTouchable || !is_touchable) ? 1.0f : 0.5f);
    {
        const char* icon = is_touchable ? ICON_FA_HAND_POINTER : ICON_FA_STOP;
        if (hoverIconButton("touchable", icon)) {
            node->flags ^= NODE_UNTOUCHABLE;
        }
    }
    ImGui::PopStyleVar();
}

void HierarchyWindow::drawEntityInTree(entity_t e, bool parentedVisible, bool parentedTouchable) {
    if (!is_entity(e)) {
        ImGui::Text("INVALID ENTITY");
        return;
    }

    ImGui::PushID(e.id);
//        ImGui::BeginGroup();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                               | ImGuiTreeNodeFlags_OpenOnDoubleClick
                               | ImGuiTreeNodeFlags_AllowItemOverlap
                               | ImGuiTreeNodeFlags_SpanFullWidth
                               | ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

    if (isSelectedInHierarchy(e)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!hasChildren(e)) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    auto nodeVisible = parentedVisible;
    auto nodeTouchable = parentedTouchable;
    auto* node = Node_get(e);
    if (node) {
        nodeVisible = nodeVisible && !(node->flags & NODE_HIDDEN);
        nodeTouchable = nodeTouchable && !(node->flags & NODE_UNTOUCHABLE);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, nodeVisible ? 0xFFFFFFFF : 0x77FFFFFF);
    if (openList.has(e.id)) {
        ImGui::SetNextItemOpen(true);
    }
    if (scrollToList.has(e.id)) {
        ImGui::SetScrollHereY();
        scrollToList.remove(e.id);
    }
    char buffer[64];
    getEntityTitle(e, buffer);
    bool opened = ImGui::TreeNodeEx("entity", flags, "%s %s", getEntityIcon(e), buffer);
    if (!opened) {
        openList.remove(e.id);
    }

    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked()) {
        selection.clear();
        selection.push_back(e.id);
    }

    drawVisibleTouchControls(node, parentedVisible, parentedTouchable);

    if (opened) {
        if (node) {
            auto it = node->child_first;
            while (it.id) {
                drawEntityInTree(it, nodeVisible, nodeTouchable);
                it = getSiblingNext(it);
            }
        }
        ImGui::TreePop();
    }

//        ImGui::EndGroup();
    ImGui::PopID();
}

void HierarchyWindow::drawEntityFiltered(entity_t e, bool parentedVisible, bool parentedTouchable) {
    if (!is_entity(e)) {
        ImGui::Text("INVALID ENTITY");
        return;
    }
    auto* node = Node_get(e);
    const char* name = node ? hsp_get(node->tag) : NULL;
    auto nodeVisible = parentedVisible;
    auto nodeTouchable = parentedTouchable;
    if (node) {
        nodeVisible = nodeVisible && !(node->flags & NODE_HIDDEN);
        nodeTouchable = nodeTouchable && !(node->flags & NODE_UNTOUCHABLE);
    }

    if (name && *name && filter.PassFilter(name)) {
        ImGui::PushID(e.id);
        ImGui::BeginGroup();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                   | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                   | ImGuiTreeNodeFlags_AllowItemOverlap
                                   | ImGuiTreeNodeFlags_SpanFullWidth
                                   | ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
        if (isSelectedInHierarchy(e)) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        // display all filtered entities just like a list
        flags |= ImGuiTreeNodeFlags_Leaf;

        ImGui::PushStyleColor(ImGuiCol_Text, nodeVisible ? 0xFFFFFFFF : 0x77FFFFFF);
        char buffer[64];
        getEntityTitle(e, buffer);
        const bool opened = ImGui::TreeNodeEx("hierarchy_node", flags, "%s%s", getEntityIcon(e), buffer);
        ImGui::PopStyleColor();

        if (ImGui::IsItemClicked()) {
            selection.clear();
            selection.push_back(e.id);
        }

        drawVisibleTouchControls(node, parentedVisible, parentedTouchable);

        if (opened) {
            ImGui::TreePop();
        }

        ImGui::EndGroup();
        ImGui::PopID();
    }

    if (node) {
        auto it = node->child_first;
        while (it.id) {
            drawEntityFiltered(it, nodeVisible, nodeTouchable);
            it = getSiblingNext(it);
        }
    }
}

void HierarchyWindow::drawFilter() {
    filter.Draw(ICON_FA_SEARCH "##hierarchy_filter", 100.0f);
    if (filter.IsActive()) {
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TIMES_CIRCLE)) {
            filter.Clear();
        }
    }
}

void HierarchyWindow::onDraw() {
    drawFilter();
    if (!is_entity(root)) {
        ImGui::TextColored({1, 0, 0, 1}, "No roots");
    } else {
        ImGui::Indent(40.0f);
        if (filter.IsActive()) {
            drawEntityFiltered(root, true, true);
        } else {
            drawEntityInTree(root, true, true);
        }
//        for(auto e3d: ecs::view<Transform3D>()) {
//            if(ecs::get<NodeName>(e3d).name == "scene 3d") {
//                drawEntityInTree(e3d, true, true);
//            }
//        }
        ImGui::Unindent(40.0f);
    }
}

// remove any invalid refs from selection
void HierarchyWindow::validateSelection() {
    unsigned i = 0;
    while (i < selection.size()) {
        if (is_entity(entity_id(selection[i]))) {
            ++i;
        } else {
            selection.erase_at(i);
        }
    }
}

void HierarchyWindow::select(entity_t e) {
    selection.clear();
    if (e.id) {
        selection.push_back(e.id);
    }
}

void HierarchyWindow::focus(entity_t e) {
    if (e.id) {
        // open parents in hierarchy
        entity_t parent = get_parent(e);
        while (parent.id) {
            openList.set(parent.id, parent);
            parent = get_parent(parent);
        }

        scrollToList.set(e.id, e);
    }
}

}