#pragma once

#include "EditorWindow.hpp"

struct ResourcesWindow : public EditorWindow {

    ResourcesWindow() {
        name = "ResourcesWindow";
        title = ICON_FA_HDD " Resources###ResourcesWindow";
    }

    ~ResourcesWindow() override = default;

    void onDraw() override;
};
