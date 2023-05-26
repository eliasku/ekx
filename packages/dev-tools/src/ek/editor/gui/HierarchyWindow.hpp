#pragma once

#include <fonts/IconsFontAwesome5.h>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/ds/Hash.hpp>

// components
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/text2d.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/viewport.h>
#include <ek/scenex/2d/camera2d.h>

struct HierarchyWindow {
    HierarchyWindow();
    ~HierarchyWindow();

    PodArray<entity_id_t> selection{};
    ImGuiTextFilter filter{};
    entity_t root = NULL_ENTITY;
    Hash<entity_t> openList{};
    Hash<entity_t> scrollToList{};

    static entity_t getSiblingNext(entity_t e);

    const char* getEntityIcon(entity_t e);

    bool isSelectedInHierarchy(entity_t e);

    static bool hasChildren(entity_t e);

    static bool hoverIconButton(const char* str_id, const char* icon);

    void drawVisibleTouchControls(node_t* node, bool parentedVisible, bool parentedTouchable);

    void drawEntityInTree(entity_t e, bool parentedVisible, bool parentedTouchable);

    void drawEntityFiltered(entity_t e, bool parentedVisible, bool parentedTouchable);

    void drawFilter();

    // remove any invalid refs from selection
    void validateSelection();

    void select(entity_t e);
    void focus(entity_t e);
};

extern HierarchyWindow editor_hierarchy_window;

void draw_hierarchy_window(void);
