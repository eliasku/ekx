#include "Editor.hpp"

#include <ek/scenex/app/basic_application.hpp>
#include <pugixml.hpp>
#include <ek/log.h>
#include <ek/assert.h>

namespace ek {

void Editor_onRenderOverlay(void* userdata) {
    g_editor->gui_.end_frame();

    bool dirty = false;
    for (auto* wnd: g_editor->windows) {
        dirty |= wnd->dirty;
    }
    if (dirty) {
        g_editor->save();
    }
}

void Editor_onUpdate(void* userdata) {
    //project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
    g_editor->gui_.begin_frame((float)g_game_app->frame_timer.dt);
    if (g_editor_config->showEditor) {
        g_editor->drawGUI();
    }
}

void Editor_onBeforeFrameBegin(void* userdata) {
    game_display* display = &g_game_app->display;
    if (g_editor_config->showEditor && !display->simulated) {
        display->simulated = true;
    } else if (!g_editor_config->showEditor && display->simulated) {
        display->simulated = false;
    }
}
void Editor_onPreRender(void* userdata) {
    g_editor->scene.onPreRender();
}

void Editor_onPostFrame(void* userdata) {
    g_editor->gui_.on_frame_completed();
    g_editor->invalidateSettings();
}


void Editor_onEvent(void* userdata, ek_app_event event) {
    EK_ASSERT(g_editor_config);
    auto& settings = *g_editor_config;
    switch (event.type) {
        case EK_APP_EVENT_KEY_DOWN:
            if (event.key.code == EK_KEYCODE_A &&
                (event.key.modifiers & EK_KEY_MOD_CONTROL) &&
                (event.key.modifiers & EK_KEY_MOD_SHIFT)) {
                settings.showEditor = !settings.showEditor;
                settings.dirty = true;
            }
            break;
        case EK_APP_EVENT_RESIZE: {
            const auto width = (int) (ek_app.viewport.width / ek_app.viewport.scale);
            const auto height = (int) (ek_app.viewport.height / ek_app.viewport.scale);
            if (width != settings.width || height != settings.height) {
                settings.width = width;
                settings.height = height;
                settings.dirty = true;
            }
        }
            break;
        default:
            break;
    }
    g_editor->gui_.on_event(event);
}

game_app_callback_t cb_Editor_onRenderOverlay;
game_app_callback_t cb_Editor_onUpdate;
game_app_callback_t cb_Editor_onBeforeFrameBegin;
game_app_callback_t cb_Editor_onPreRender;
game_app_callback_t cb_Editor_onPostFrame;
game_app_callback_t cb_Editor_onEvent;

Editor::Editor() {
    windows.push_back(&scene);
    windows.push_back(&game);
    windows.push_back(&inspector);
    windows.push_back(&hierarchy);
    // project
    windows.push_back(&console);
    windows.push_back(&stats);
    windows.push_back(&resources);
    windows.push_back(&memory);

    load();

    cb_Editor_onRenderOverlay.fn = (void*)Editor_onRenderOverlay;
    cb_Editor_onUpdate.fn = (void*)Editor_onUpdate;
    cb_Editor_onBeforeFrameBegin.fn = (void*)Editor_onBeforeFrameBegin;
    cb_Editor_onPreRender.fn = (void*)Editor_onPreRender;
    cb_Editor_onPostFrame.fn = (void*)Editor_onPostFrame;
    cb_Editor_onEvent.fn = (void*)Editor_onEvent;

    game_app_add_callback(&game_app_callbacks.on_render_overlay, &cb_Editor_onRenderOverlay);
    game_app_add_callback(&game_app_callbacks.on_update, &cb_Editor_onUpdate);
    game_app_add_callback(&game_app_callbacks.on_before_frame_begin, &cb_Editor_onBeforeFrameBegin);
    game_app_add_callback(&game_app_callbacks.on_pre_render, &cb_Editor_onPreRender);
    game_app_add_callback(&game_app_callbacks.on_post_frame, &cb_Editor_onPostFrame);
    game_app_add_callback(&game_app_callbacks.on_event, &cb_Editor_onEvent);
}

Editor::~Editor() = default;

void Editor::load() {
    log_debug("load editor layout state");
    pugi::xml_document doc{};
    if (!doc.load_file("EditorLayoutState.xml")) {
        return;
    }
    auto node = doc.first_child();
    for (auto* wnd: windows) {
        wnd->load(node);
    }
}

void Editor::save() {
    pugi::xml_document xml;
    auto node = xml.append_child("EditorLayoutState");
    for (auto* wnd: windows) {
        wnd->save(node);
    }
    xml.save_file("EditorLayoutState.xml");
}



const char* editorSettingsPath = "editor_settings.xml";

void EditorSettings::save() const {
    pugi::xml_document xml;

    auto node = xml.append_child("editor");
    node.append_attribute("notifyAssetsOnScaleFactorChanged").set_value(notifyAssetsOnScaleFactorChanged);
    node.append_attribute("showEditor").set_value(showEditor);
    auto wnd = node.append_child("window");
    wnd.append_attribute("width").set_value(width);
    wnd.append_attribute("height").set_value(height);
    if (!xml.save_file(editorSettingsPath)) {
        log_error("Can't save editor settings");
    }
}

void EditorSettings::load() {
    log_debug("loading editor settings");
    pugi::xml_document doc{};
    if (!doc.load_file(editorSettingsPath)) {
        log_error("Can't load editor settings");
        return;
    }
    auto node = doc.first_child();
    notifyAssetsOnScaleFactorChanged = node.attribute("notifyAssetsOnScaleFactorChanged").as_bool(
            notifyAssetsOnScaleFactorChanged);
    showEditor = node.attribute("showEditor").as_bool(showEditor);
    auto wnd = node.child("window");
    width = wnd.attribute("width").as_int((int) ek_app.config.width);
    height = wnd.attribute("height").as_int((int) ek_app.config.height);
}

void Editor::invalidateSettings() {
    auto& settings = *g_editor_config;
    if (settings.dirty) {
        ++settings.auto_save_counter;
        if (settings.auto_save_counter > 20) {
            settings.auto_save_counter = 0;
            settings.save();
            settings.dirty = false;
        }
    }
}

}

ek::Editor* g_editor = nullptr;
void init_editor(void) {
    EK_ASSERT(!g_editor);
    g_editor = new ek::Editor();
}

ek::EditorSettings* g_editor_config = nullptr;
void init_editor_config(void) {
    g_editor_config = new ek::EditorSettings();
    g_editor_config->load();
}
