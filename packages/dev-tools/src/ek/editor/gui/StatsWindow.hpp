#pragma once

#include "EditorWindow.hpp"

struct StatsWindow : public EditorWindow {

    StatsWindow() {
        name = "StatsWindow";
        title = ICON_FA_TACHOMETER_ALT " Stats###StatsWindow";
    }

    ~StatsWindow() override = default;

    void onDraw() override;
};
