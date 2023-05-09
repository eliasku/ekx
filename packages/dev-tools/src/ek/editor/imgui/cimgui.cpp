#include <ek/editor/imgui/cimgui.h>
#include <ek/editor/imgui/imgui.hpp>

bool ImGui_Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    ImGui::Begin(name, p_open, flags);
}

void ImGui_End(void) {
    ImGui::End();
}

bool ImGui_Button(const char* name) {
    ImGui::Button(name);
}

void ImGui_PopStyleVar(int count) {
    ImGui::PopStyleVar(count ? count : 1);
}

void ImGui_PushStyleVar(int var, float v) {
    ImGui::PushStyleVar(var, v);
}

void ImGui_PushStyleVar_f2(int var, vec2_t v) {
    ImGui::PushStyleVar(var, *(ImVec2*)&v);
}

bool ImGui_TreeNode(const void* ptr_id, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool is_open = ImGui::TreeNodeExV(ptr_id, 0, fmt, args);
    va_end(args);
    return is_open;
}

void ImGui_TreePop(void) {
    ImGui::TreePop();
}

void ImGui_PushID(int id) {
    ImGui::PushID(id);
}

void ImGui_PopID(void) {
    ImGui::PopID();
}

void ImGui_Separator(void) {
    ImGui_Separator();
}
