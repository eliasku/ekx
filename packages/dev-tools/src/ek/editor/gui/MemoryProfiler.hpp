#pragma once

#include "EditorWindow.hpp"

struct MemoryProfiler : public EditorWindow {

    MemoryProfiler() {
        name = "MemoryProfiler";
        title = ICON_FA_MEMORY " Memory###MemoryProfiler";
    }

    ~MemoryProfiler() override = default;

    void onDraw() override;
};
