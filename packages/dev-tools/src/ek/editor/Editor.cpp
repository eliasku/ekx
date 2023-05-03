#include "Editor.hpp"
#include "editor_api.h"

#include <ek/log.h>
#include <ek/scenex/app/base_game.h>
#include <pugixml.hpp>

#define EDITOR_CONFIG_PATH "editor_config.xml"
#define EDITOR_LAYOUT_PATH "editor_layout.xml"

static void editor_load_layout(void) {
    log_debug("load editor layout state");
    pugi::xml_document doc{};
    if (!doc.load_file(EDITOR_LAYOUT_PATH)) {
        return;
    }
    auto node = doc.first_child();
    for (auto* wnd: g_editor.windows) {
        wnd->load(node);
    }
}

static void editor_save_layout(void) {
    pugi::xml_document xml;
    auto node = xml.append_child("EditorLayoutState");
    for (auto* wnd: g_editor.windows) {
        wnd->save(node);
    }
    xml.save_file(EDITOR_LAYOUT_PATH);
}

static void editor_save_config(void) {
    pugi::xml_document xml;

    pugi::xml_node node = xml.append_child("editor");
    node.append_attribute("hidden").set_value(g_editor.config.hidden);
    node.append_attribute("width").set_value(g_editor.config.width);
    node.append_attribute("height").set_value(g_editor.config.height);
    if (!xml.save_file(EDITOR_CONFIG_PATH)) {
        log_error("Can't save editor settings");
    }
}

static void editor_load_config(void) {
    log_debug("loading editor settings");
    pugi::xml_document doc{};
    if (!doc.load_file(EDITOR_CONFIG_PATH)) {
        log_error("Can't load editor settings");
        return;
    }
    pugi::xml_node node = doc.first_child();
    g_editor.config.hidden = node.attribute("hidden").as_bool(g_editor.config.hidden);
    g_editor.config.width = node.attribute("width").as_int((int) ek_app.config.width);
    g_editor.config.height = node.attribute("height").as_int((int) ek_app.config.height);
}

static void invalidate_config(void) {
    if (g_editor.config.dirty) {
        ++g_editor.config.dirty;
        if (g_editor.config.dirty > 20) {
            g_editor.config.dirty = 0;
            editor_save_config();
        }
    }
}

void editor_render_overlay(void) {
    ekimgui_end_frame();

    bool dirty = false;
    for (EditorWindow* wnd: g_editor.windows) {
        dirty |= wnd->dirty;
    }
    if (dirty) {
        editor_save_layout();
    }
}

void editor_update(void) {
    //project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
    ekimgui_begin_frame((float) game_app_state.frame_timer.dt);
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
    g_editor.scene.pre_render();
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
            const int width = (int) (ek_app.viewport.width / ek_app.viewport.scale);
            const int height = (int) (ek_app.viewport.height / ek_app.viewport.scale);
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

Editor g_editor;

void editor_setup(void) {
    g_editor.windows.push_back(&g_editor.scene);
    g_editor.windows.push_back(&g_editor.game);
    g_editor.windows.push_back(&g_editor.inspector);
    g_editor.windows.push_back(&g_editor.hierarchy);
    g_editor.windows.push_back(&g_editor.console);
    g_editor.windows.push_back(&g_editor.stats);
    g_editor.windows.push_back(&g_editor.resources);
    g_editor.windows.push_back(&g_editor.memory);
    ekimgui_setup();
    editor_load_layout();
}

void editor_load(void) {
    g_editor.config = {};
    editor_load_config();
    if (g_editor.config.width > 0 && g_editor.config.height > 0) {
        ek_app.config.window_width = g_editor.config.width;
        ek_app.config.window_height = g_editor.config.height;
    }
}
