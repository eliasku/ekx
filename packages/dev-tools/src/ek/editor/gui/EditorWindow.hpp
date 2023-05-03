#pragma once

#include "../imgui/imgui.hpp"
#include <pugixml.hpp>
#include <fonts/IconsFontAwesome5.h>

struct EditorWindow {
    const char* name;
    const char* title;
    bool dirty = false;
    bool opened = true;
    bool full_frame = false;

    virtual ~EditorWindow() = default;

    virtual void onDraw() {}

    virtual void onLoad(const pugi::xml_node& xml) { (void)xml; }

    virtual void onSave(pugi::xml_node& xml) { (void)xml; }

    void load(const pugi::xml_node& xml) {
        if (!name || !*name) return;
        auto node = xml.child(name);
        if (!node.empty()) {
            opened = node.attribute("opened").as_bool(false);
            onLoad(node);
        }
        dirty = false;
    }

    void save(pugi::xml_node& xml) {
        if (!name || !*name) return;
        auto node = xml.child(name);
        if (node.empty()) {
            node = xml.append_child(name);
        }
        node.append_attribute("opened").set_value(opened);
        onSave(node);
        dirty = false;
    }

    void show() {
        if (!opened) {
            return;
        }

        int flags = 0;
        if (full_frame) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});
            flags |= ImGuiWindowFlags_NoBackground;
        }

        if (ImGui::Begin(title, &opened, flags)) {
            onDraw();
            if (!opened) {
                dirty = true;
            }
        }
        ImGui::End();

        if (full_frame) {
            ImGui::PopStyleVar();
        }
    }

    void mainMenu() {
        dirty |= ImGui::MenuItem(title, nullptr, &opened);
    }
};
