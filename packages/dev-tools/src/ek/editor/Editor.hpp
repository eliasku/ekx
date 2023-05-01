#pragma once

#include "imgui/ekimgui.h"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/StatsWindow.hpp"
#include "gui/GameWindow.hpp"
#include "gui/MemoryProfiler.hpp"
#include "gui/SceneWindow.hpp"
#include "gui/ConsoleWindow.hpp"
#include "gui/ResourcesWindow.hpp"

struct EditorConfig {
    int dirty;
    int width;
    int height;
    bool hidden;
};

struct Editor {
    ek::HierarchyWindow hierarchy{};
    ek::InspectorWindow inspector{};
    ek::GameWindow game{};
    ek::StatsWindow stats{};
    ek::MemoryProfiler memory{};
    ek::SceneWindow scene{};
    ek::ConsoleWindow console{};
    ek::ResourcesWindow resources{};
    ek::PodArray<ek::EditorWindow*> windows{};
    EditorConfig config{};
};

void editor_draw_gui(void);

extern Editor g_editor;
