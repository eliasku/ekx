#include <ek/editor/imgui/cimgui.h>
#include <ek/editor/imgui/imgui.hpp>

bool ImGui_Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    return ImGui::Begin(name, p_open, flags);
}

void ImGui_End(void) {
    ImGui::End();
}

bool ImGui_Button(const char* name) {
    return ImGui::Button(name);
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

bool ImGui_TreeNodeEx(const void* ptr_id, int flags, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool is_open = ImGui::TreeNodeExV(ptr_id, flags, fmt, args);
    va_end(args);
    return is_open;
}

void ImGui_TreePop(void) {
    ImGui::TreePop();
}

void ImGui_PushID(uintptr_t id) {
    ImGui::PushID((const void*)id);
}

void ImGui_PopID(void) {
    ImGui::PopID();
}

void ImGui_Separator(void) {
    ImGui::Separator();
}

void ImGui_BeginGroup(void) {
    ImGui::BeginGroup();
}

void ImGui_EndGroup(void) {
    ImGui::EndGroup();
}

void ImGui_TextUnformatted(const char* text, const char* text_end) {
    ImGui::TextUnformatted(text, text_end);
}

void ImGui_Text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void ImGui_TextColored(vec4_t color, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(*(ImVec4*)&color, fmt, args);
    va_end(args);
}

void ImGui_TextDisabled(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::TextDisabledV(fmt, args);
    va_end(args);
}

bool ImGui_BeginTabBar(const char* str_id, int flags) {
    return ImGui::BeginTabBar(str_id, flags);
}

void ImGui_EndTabBar(void) {
    ImGui::EndTabBar();
}

bool ImGui_BeginTabItem(const char* label, bool* p_open, int flags) {
    return ImGui::BeginTabItem(label, p_open, flags);
}

void ImGui_EndTabItem(void) {
    ImGui::EndTabItem();
}

void ImGui_BeginDisabled(bool disabled) {
    ImGui::BeginDisabled(disabled);
}

void ImGui_EndDisabled(void) {
    ImGui::EndDisabled();
}

