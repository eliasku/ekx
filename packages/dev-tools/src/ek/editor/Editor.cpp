#include "Editor.hpp"
#include "editor_api.h"

#include <calo.h>
#include <ek/editor/imgui/cimgui.h>
#include <ek/log.h>
#include <ek/scenex/app/base_game.h>

void show_editor_window(editor_wnd_t* wnd) {
    if (!wnd->opened) {
        return;
    }

    int flags = 0;
    if (wnd->full_frame) {
        ImGui_PushStyleVar_f2(ImGuiStyleVar_WindowPadding, vec2(0, 0));
        flags |= ImGuiWindowFlags_NoBackground;
    }

    if (ImGui_Begin(wnd->title, &wnd->opened, flags)) {
        if (wnd->draw) {
            wnd->draw();
        }
        if (!wnd->opened) {
            g_editor.config.dirty |= 1;
        }
    }
    ImGui_End();

    if (wnd->full_frame) {
        ImGui_PopStyleVar(0);
    }
}

#define EDITOR_STATE_PATH "editor.bin"

static void editor_load(void) {
    log_debug("load editor state");
    //uint32_t size;
    calo_reader_t r = INIT_ZERO;
    void* data = read_file(EDITOR_STATE_PATH, NULL);
    if (data) {
        r.p = (uint8_t*)data;
        g_editor.config.width = read_u32(&r);
        g_editor.config.height = read_u32(&r);
        g_editor.config.hidden = read_u32(&r) != 0;

        for (int i = 0; i < EDITOR_WINDOWS_NUM; ++i) {
            editor_wnd_t* wnd = &g_editor.windows[i];
            wnd->opened = read_u32(&r) != 0;
            if (wnd->load) {
                wnd->load(&r);
            }
        }
        free(data);
    }
    else {
        for (int i = 0; i < EDITOR_WINDOWS_NUM; ++i) {
            editor_wnd_t* wnd = &g_editor.windows[i];
            wnd->opened = true;
            if (wnd->load) {
                wnd->load(NULL);
            }
        }
    }
}

static void editor_save(void) {
    calo_writer_t w = new_writer(1);
    write_u32(&w, g_editor.config.width);
    write_u32(&w, g_editor.config.height);
    write_u32(&w, g_editor.config.hidden ? 1 : 0);
    for (int i = 0; i < EDITOR_WINDOWS_NUM; ++i) {
        editor_wnd_t* wnd = &g_editor.windows[i];
        write_u32(&w, wnd->opened ? 1 : 0);
        if (wnd->save) {
            wnd->save(&w);
        }
    }
    FILE* f = fopen(EDITOR_STATE_PATH, "wb");
    if (f) {
        fwrite(w.data, 1, w.p - (uint8_t*)w.data, f);
        fclose(f);
    }
    free_writer(&w);
}

static void invalidate_config(void) {
    if (g_editor.config.dirty) {
        ++g_editor.config.dirty;
        if (g_editor.config.dirty > 20) {
            g_editor.config.dirty = 0;
            editor_save();
        }
    }
}

void editor_render_overlay(void) {
    ekimgui_end_frame();
}

void editor_update(void) {
    //project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
    ekimgui_begin_frame((float)game_app_state.frame_timer.dt);
    if (!g_editor.config.hidden) {
        editor_draw_gui();
    }
}

void editor_before_frame_begin(void) {
    game_display* display = &game_app_state.display;
    if (!g_editor.config.hidden && !display->simulated) {
        display->simulated = true;
    } else if (g_editor.config.hidden && display->simulated) {
        display->simulated = false;
    }
}

void editor_pre_render(void) {
    editor_scene_window.pre_render();
}

void editor_post_frame(void) {
    ekimgui_on_frame_completed();
    invalidate_config();
}

void editor_event(ek_app_event event) {
    switch (event.type) {
        case EK_APP_EVENT_KEY_DOWN:
            if (event.key.code == EK_KEYCODE_A &&
                (event.key.modifiers & EK_KEY_MOD_CONTROL) &&
                (event.key.modifiers & EK_KEY_MOD_SHIFT)) {
                g_editor.config.hidden = !g_editor.config.hidden;
                g_editor.config.dirty |= 1;
            }
            break;
        case EK_APP_EVENT_RESIZE: {
            const int width = (int)(ek_app.viewport.width / ek_app.viewport.scale);
            const int height = (int)(ek_app.viewport.height / ek_app.viewport.scale);
            if (width != g_editor.config.width || height != g_editor.config.height) {
                g_editor.config.width = width;
                g_editor.config.height = height;
                g_editor.config.dirty |= 1;
            }
        } break;
        default:
            break;
    }
    ekimgui_on_event(event);
}

editor_t g_editor;

void editor_setup(void) {
    ekimgui_setup();
}

void editor_init(void) {
    g_editor.windows[EDITOR_SCENE].name = "SceneWindow";
    g_editor.windows[EDITOR_SCENE].title = ICON_FA_GLOBE " Scene###SceneWindow";
    g_editor.windows[EDITOR_SCENE].full_frame = true;
    g_editor.windows[EDITOR_SCENE].draw = draw_scene_window;

    g_editor.windows[EDITOR_GAME].name = "GameWindow";
    g_editor.windows[EDITOR_GAME].title = ICON_FA_GAMEPAD " Game###GameWindow";
    g_editor.windows[EDITOR_GAME].full_frame = true;
    g_editor.windows[EDITOR_GAME].load = game_window_load;
    g_editor.windows[EDITOR_GAME].save = game_window_save;
    g_editor.windows[EDITOR_GAME].draw = game_window_draw;

    g_editor.windows[EDITOR_INSPECTOR].name = "InspectorWindow";
    g_editor.windows[EDITOR_INSPECTOR].title = ICON_FA_PUZZLE_PIECE " Inspector###InspectorWindow";
    g_editor.windows[EDITOR_INSPECTOR].draw = draw_inspector_window;

    g_editor.windows[EDITOR_HIERARCHY].name = "HierarchyWindow";
    g_editor.windows[EDITOR_HIERARCHY].title = ICON_FA_SITEMAP " Hierarchy###HierarchyWindow";
    g_editor.windows[EDITOR_HIERARCHY].draw = draw_hierarchy_window;

    g_editor.windows[EDITOR_CONSOLE].name = "ConsoleWindow";
    g_editor.windows[EDITOR_CONSOLE].title = ICON_FA_LAPTOP_CODE " Console###ConsoleWindow";
    g_editor.windows[EDITOR_CONSOLE].draw = show_console;

    g_editor.windows[EDITOR_STATS].name = "StatsWindow";
    g_editor.windows[EDITOR_STATS].title = ICON_FA_TACHOMETER_ALT " Stats###StatsWindow";
    g_editor.windows[EDITOR_STATS].draw = draw_stats_window;

    g_editor.windows[EDITOR_RESOURCES].name = "ResourcesWindow";
    g_editor.windows[EDITOR_RESOURCES].title = ICON_FA_HDD " Resources###ResourcesWindow";
    g_editor.windows[EDITOR_RESOURCES].draw = draw_resources_window;

    g_editor.windows[EDITOR_MEMORY].name = "MemoryProfiler";
    g_editor.windows[EDITOR_MEMORY].title = ICON_FA_MEMORY " Memory###MemoryProfiler";
    g_editor.windows[EDITOR_MEMORY].draw = draw_memory_profiler;

    init_console();

    editor_load();

    if (g_editor.config.width > 0 && g_editor.config.height > 0) {
        ek_app.config.window_width = g_editor.config.width;
        ek_app.config.window_height = g_editor.config.height;
    }
}
