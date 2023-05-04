#pragma once

#include "EditorWindow.hpp"

struct InspectorWindow : public EditorWindow {
    PodArray<entity_id_t> list{};

    InspectorWindow() {
        name = "InspectorWindow";
        title = ICON_FA_PUZZLE_PIECE " Inspector###InspectorWindow";
    }

    ~InspectorWindow() override = default;

    void gui_inspector(entity_t entity);

    void onDraw() override;
};
