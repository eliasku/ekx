#pragma once

#include "../imgui/cimgui.h"
#include <ek/math.h>
#include <ecx/ecx.h>

bool ImGui_EditRect(const char* label, rect_t* p_rect, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

#ifdef __cplusplus
extern "C" {
#endif

bool ImGui_ToolbarButton(const char* label, bool active, const char* tooltip);

bool ImGui_InputText(const char* label, void** p_str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userdata);

bool ImGui_InputTextMultiline(const char* label, void** p_str, vec2_t size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userdata);

void ImGui_HelpMarker(const char* desc);

bool ImGui_Color32Edit(const char* label, color_t* color);

typedef struct font_ font_t;
typedef struct sprite_ sprite_t;
typedef struct text_layer_effect_ text_layer_effect_t;

void get_debug_node_path(entity_t e, char buffer[1024]);

void gui_text_layer_effect(text_layer_effect_t* layer);

void gui_sprite(const sprite_t* sprite);

void gui_font(const font_t* font);

#ifdef __cplusplus
}
#endif
