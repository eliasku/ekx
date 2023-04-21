#pragma once

#include <ek/scenex/assets/Asset.hpp>
#include "imgui/ImGuiIntegration.hpp"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/StatsWindow.hpp"
#include "gui/GameWindow.hpp"
#include "gui/MemoryProfiler.hpp"
#include "gui/SceneWindow.hpp"
#include "gui/ConsoleWindow.hpp"
#include "gui/ResourcesWindow.hpp"

namespace ek {

struct EditorSettings {
    int auto_save_counter = 0;

    bool notifyAssetsOnScaleFactorChanged = true;
    bool showEditor = true;
    int width = 0;
    int height = 0;
    bool dirty = false;

    void save() const;

    void load();
};

struct Editor {
    Editor();

    ~Editor();

    void drawGUI();

    HierarchyWindow hierarchy{};
    InspectorWindow inspector{};
    GameWindow game{};
    StatsWindow stats{};
    MemoryProfiler memory{};
    SceneWindow scene{};
    ConsoleWindow console{};
    ResourcesWindow resources{};
    PodArray<EditorWindow*> windows{};

    ImGuiIntegration gui_;

    void load();
    void save();

    static void invalidateSettings();
};

}

extern ek::EditorSettings* g_editor_config;
extern ek::Editor* g_editor;
