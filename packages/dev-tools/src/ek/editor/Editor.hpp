#pragma once

#include "gui/ConsoleWindow.hpp"
#include "gui/GameWindow.hpp"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/MemoryProfiler.hpp"
#include "gui/ResourcesWindow.hpp"
#include "gui/SceneWindow.hpp"
#include "gui/StatsWindow.hpp"
#include "imgui/ekimgui.h"

struct EditorConfig {
    int dirty;
    int width;
    int height;
    bool hidden;
};

struct Editor {
    HierarchyWindow hierarchy{};
    InspectorWindow inspector{};
    GameWindow game{};
    StatsWindow stats{};
    MemoryProfiler memory{};
    SceneWindow scene{};
    ConsoleWindow console{};
    ResourcesWindow resources{};
    PodArray<EditorWindow*> windows{};
    EditorConfig config{};
};

void editor_draw_gui(void);

extern Editor g_editor;
