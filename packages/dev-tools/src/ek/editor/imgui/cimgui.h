#ifndef EK_CIMGUI_H
#define EK_CIMGUI_H

#include <ek/math.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t ImGuiWindowFlags;

enum imgui_stylevar_
{
    // Enum name --------------------- // Member in ImGuiStyle structure (see ImGuiStyle for descriptions)
    ImGui_StyleVar_Alpha,               // float     Alpha
    ImGui_StyleVar_DisabledAlpha,       // float     DisabledAlpha
    ImGui_StyleVar_WindowPadding,       // ImVec2    WindowPadding
    ImGui_StyleVar_WindowRounding,      // float     WindowRounding
    ImGui_StyleVar_WindowBorderSize,    // float     WindowBorderSize
    ImGui_StyleVar_WindowMinSize,       // ImVec2    WindowMinSize
    ImGui_StyleVar_WindowTitleAlign,    // ImVec2    WindowTitleAlign
    ImGui_StyleVar_ChildRounding,       // float     ChildRounding
    ImGui_StyleVar_ChildBorderSize,     // float     ChildBorderSize
    ImGui_StyleVar_PopupRounding,       // float     PopupRounding
    ImGui_StyleVar_PopupBorderSize,     // float     PopupBorderSize
    ImGui_StyleVar_FramePadding,        // ImVec2    FramePadding
    ImGui_StyleVar_FrameRounding,       // float     FrameRounding
    ImGui_StyleVar_FrameBorderSize,     // float     FrameBorderSize
    ImGui_StyleVar_ItemSpacing,         // ImVec2    ItemSpacing
    ImGui_StyleVar_ItemInnerSpacing,    // ImVec2    ItemInnerSpacing
    ImGui_StyleVar_IndentSpacing,       // float     IndentSpacing
    ImGui_StyleVar_CellPadding,         // ImVec2    CellPadding
    ImGui_StyleVar_ScrollbarSize,       // float     ScrollbarSize
    ImGui_StyleVar_ScrollbarRounding,   // float     ScrollbarRounding
    ImGui_StyleVar_GrabMinSize,         // float     GrabMinSize
    ImGui_StyleVar_GrabRounding,        // float     GrabRounding
    ImGui_StyleVar_TabRounding,         // float     TabRounding
    ImGui_StyleVar_ButtonTextAlign,     // ImVec2    ButtonTextAlign
    ImGui_StyleVar_SelectableTextAlign, // ImVec2    SelectableTextAlign
    ImGui_StyleVar_SeparatorTextBorderSize,// float  SeparatorTextBorderSize
    ImGui_StyleVar_SeparatorTextAlign,  // ImVec2    SeparatorTextAlign
    ImGui_StyleVar_SeparatorTextPadding,// ImVec2    SeparatorTextPadding
    ImGui_StyleVar_COUNT
};

bool ImGui_Begin(const char* name, bool* p_open, ImGuiWindowFlags flags);
void ImGui_End(void);
bool ImGui_Button(const char* name);

// use 0 for 1
void ImGui_PopStyleVar(int count);
void ImGui_PushStyleVar(int var, float v);
void ImGui_PushStyleVar_f2(int var, vec2_t v);

bool ImGui_TreeNode(const void* ptr_id, const char* fmt, ...);
bool ImGui_TreeNodeEx(const void* ptr_id, int flags, const char* fmt, ...);
void ImGui_TreePop(void);

void ImGui_BeginGroup(void);
void ImGui_EndGroup(void);

void ImGui_PushID(uintptr_t id);
void ImGui_PopID(void);

void ImGui_Separator(void);

void ImGui_TextUnformatted(const char* text, const char* text_end);
void ImGui_Text(const char* fmt, ...);
void ImGui_TextColored(vec4_t color, const char* fmt, ...);
void ImGui_TextDisabled(const char* fmt, ...);
#define ImGui_TextError(...) ImGui_TextColored(vec4(1,0,0,1),__VA_ARGS__)

bool ImGui_BeginTabBar(const char* str_id, int flags);
void ImGui_EndTabBar(void);

bool ImGui_BeginTabItem(const char* label, bool* p_open, int flags);
void ImGui_EndTabItem(void);

void ImGui_BeginDisabled(bool disabled);
void ImGui_EndDisabled(void);

#ifdef __cplusplus
}
#endif

#endif // EK_CIMGUI_H
