#pragma once

#include <ek/scenex/asset2/Asset.hpp>
#include "imgui/ImGuiIntegration.hpp"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/StatsWindow.hpp"
#include "gui/GameWindow.hpp"
#include "gui/MemoryProfiler.hpp"
#include "gui/SceneWindow.hpp"
#include "gui/ConsoleWindow.hpp"
#include "gui/ResourcesWindow.hpp"
#include <ek/scenex/app/GameAppListener.hpp>

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

class basic_application;

class Editor : public GameAppListener {
public:
    Editor();

    ~Editor() override;

    void onEvent(const ek_app_event& event) override;
    void onPostFrame() override;

public:
    // GameApp callbacks
    void onRenderOverlay() override;
    void onRenderFrame() override;
    void onUpdate() override;
    void onBeforeFrameBegin() override;
    void onPreRender() override;

    void drawGUI();

    static void invalidateSettings();

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
};

}

extern ek::EditorSettings* g_editor_config;
void init_editor_config(void);

extern ek::Editor* g_editor;
void init_editor(void);
