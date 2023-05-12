#include <ek/editor/Editor.hpp>

#include <ek/scenex/app/base_game.h>
#include <ek/editor/imgui/ekimgui.h>
#include <ek/editor/imgui/cimgui.h>
#include <ek/editor/imgui/imgui.hpp>
#include <ekx/app/input_state.h>

// private impls:
#include "StatsWindow_impl.hpp"
#include "GameWindow_impl.hpp"
#include "HierarchyWindow_impl.hpp"
#include "InspectorWindow_impl.hpp"
#include "MemoryProfiler_impl.hpp"
#include "SceneWindow_impl.hpp"
#include "console.cpp.h"
#include "ResourcesWindow_impl.hpp"
#include "Widgets_impl.hpp"

#ifdef EK_DEV_TOOLS_DEBUG

#include "../helpers/test_window.hpp"
#include "../helpers/font_icons_preview.hpp"

#endif

#ifdef EK_DEV_TOOLS_DEBUG
bool plot_demo_f;
bool gui_demo_f;
bool gui_metrics_f;
bool gui_debug_log_f;
bool gui_stack_tool_f;
bool gui_about_f;
bool font_icons_preview_f;
bool test_window_f;
#endif

void editor_draw_gui(void) {
    // TODO: 2d/3d root
    editor_hierarchy_window.root = game_app_state.root;

    bool resetLayout = false;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Emulate Touch Input", nullptr, &g_input_state.emulate_touch);
            ///
            ImGui_Separator();
            ImGui_Text("User Insets Absolute");
            ImGui::DragFloat4("##userInsetsAbsolute",
                              game_app_state.display.info.user_insets_abs.data);
            ImGui_Text("User Insets Relative");
            ImGui::SliderFloat4("##userInsetsRelative",
                                game_app_state.display.info.user_insets_rel.data, 0.0f, 1.0f);

            ImGui_Separator();

            if (ImGui::BeginMenu(ICON_FA_FEATHER_ALT " Sokol")) {
                ImGui::MenuItem("Capabilities", NULL, &sokol_gfx_gui_state.caps.open);
                ImGui::MenuItem("Buffers", NULL, &sokol_gfx_gui_state.buffers.open);
                ImGui::MenuItem("Images", NULL, &sokol_gfx_gui_state.images.open);
                ImGui::MenuItem("Shaders", NULL, &sokol_gfx_gui_state.shaders.open);
                ImGui::MenuItem("Pipelines", NULL, &sokol_gfx_gui_state.pipelines.open);
                ImGui::MenuItem("Passes", NULL, &sokol_gfx_gui_state.passes.open);
                ImGui::MenuItem("Calls", NULL, &sokol_gfx_gui_state.capture.open);
                ImGui::EndMenu();
            }

#ifdef EK_DEV_TOOLS_DEBUG
            if (ImGui::BeginMenu(ICON_FA_ICE_CREAM " Dear ImGui")) {
                ImGui::MenuItem("Gui Demo", NULL, &gui_demo_f);
                ImGui::MenuItem("Plot Demo", NULL, &plot_demo_f);
                ImGui::MenuItem("Metrics", NULL, &gui_metrics_f);
                ImGui::MenuItem("Debug Log", NULL, &gui_debug_log_f);
                ImGui::MenuItem("Stack Tool", NULL, &gui_stack_tool_f);
                ImGui::MenuItem("About", NULL, &gui_about_f);
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Font Icons", NULL, &font_icons_preview_f);
            ImGui::MenuItem("Test", NULL, &test_window_f);
#endif

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            for (int i = 0; i < EDITOR_WINDOWS_NUM; ++i) {
                editor_wnd_t* wnd = &g_editor.windows[i];
                g_editor.config.dirty |= ImGui::MenuItem(wnd->title, NULL, &wnd->opened);
            }
            if (ImGui::MenuItem("Reset Layout")) {
                resetLayout = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::EndMenu();
        }
        ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.0f) -
                        (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)));

        if (ImGui::Button(g_editor.game.paused ? ICON_FA_PLAY : ICON_FA_PAUSE)) {
            g_editor.game.paused = !g_editor.game.paused;
            g_editor.config.dirty |= 1;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        g_editor.config.dirty |= ImGui::SliderFloat(ICON_FA_CLOCK, &g_editor.game.time_scale, 0.0f, 3.0f, "%.3f", 1.0f);
        ImGui::SameLine();
        g_editor.config.dirty |= ImGui::Checkbox(ICON_FA_STOPWATCH, &g_editor.game.profiler);

        s_profile_metrics.enabled = g_editor.game.profiler;
        g_time_layers[TIME_LAYER_ROOT].scale = g_editor.game.paused ? 0.0f : g_editor.game.time_scale;

        ImGui::EndMainMenuBar();
    }
    ImGuiID dockSpaceId = ImGui::DockSpaceOverViewport();
    if (resetLayout) {
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId);
        ImVec2 size;
        size.x = ImGui::GetIO().DisplaySize.x * ImGui::GetIO().DisplayFramebufferScale.x;
        size.y = ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y;
        ImGui::DockBuilderSetNodeSize(dockSpaceId, size);
        ImGuiID DockBottom = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.3f, nullptr, &dockSpaceId);
        ImGuiID DockLeft = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.2f, nullptr, &dockSpaceId);
        ImGuiID DockRight = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.20f, nullptr, &dockSpaceId);

        ImGuiID DockLeftChild = ImGui::DockBuilderSplitNode(DockLeft, ImGuiDir_Down, 0.875f, nullptr, &DockLeft);
        ImGuiID DockRightChild = ImGui::DockBuilderSplitNode(DockRight, ImGuiDir_Down, 0.875f, nullptr, &DockRight);
        ImGuiID DockingLeftDownChild = ImGui::DockBuilderSplitNode(DockLeftChild, ImGuiDir_Down, 0.06f, nullptr,
                                                                   &DockLeftChild);
        ImGuiID DockingRightDownChild = ImGui::DockBuilderSplitNode(DockRightChild, ImGuiDir_Down, 0.06f, nullptr,
                                                                    &DockRightChild);

        ImGuiID DockBottomChild = ImGui::DockBuilderSplitNode(DockBottom, ImGuiDir_Down, 0.2f, nullptr, &DockBottom);
        ImGuiID DockingBottomLeftChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Left, 0.5f, nullptr,
                                                                     &DockBottomChild);
        ImGuiID DockingBottomRightChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Right, 0.5f, nullptr,
                                                                      &DockBottomChild);

        ImGuiID DockMiddle = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 1.0f, nullptr, &dockSpaceId);

        ImGui::DockBuilderDockWindow("###SceneWindow", DockMiddle);
        ImGui::DockBuilderDockWindow("###GameWindow", DockMiddle);

        ImGui::DockBuilderDockWindow("###HierarchyWindow", DockLeft);
        ImGui::DockBuilderDockWindow("###InspectorWindow", DockRight);
        ImGui::DockBuilderDockWindow("###ConsoleWindow", DockingBottomLeftChild);
    }

    for (int i = 0; i < EDITOR_WINDOWS_NUM; ++i) {
        show_editor_window(&g_editor.windows[i]);
    }

    editor_hierarchy_window.validateSelection();
    inspector_list = editor_hierarchy_window.selection;

#ifdef EK_DEV_TOOLS_DEBUG
    if (gui_demo_f) ImGui::ShowDemoWindow(&gui_demo_f);
    if (gui_metrics_f) ImGui::ShowMetricsWindow(&gui_metrics_f);
    if (gui_debug_log_f) ImGui::ShowDebugLogWindow(&gui_debug_log_f);
    if (gui_stack_tool_f) ImGui::ShowStackToolWindow(&gui_stack_tool_f);
    if (gui_about_f) ImGui::ShowAboutWindow(&gui_about_f);
    if (plot_demo_f) ImPlot::ShowDemoWindow(&plot_demo_f);

    if (font_icons_preview_f) font_icons_preview(&font_icons_preview_f);
    if (test_window_f) test_window(&test_window_f);
#endif // EK_DEV_TOOLS_SLIM
}

