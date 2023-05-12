#pragma once

#include <ek/scenex/2d/sprite.h>

typedef struct font_ font_t;

void ImGui_HelpMarker(const char* desc);

bool ImGui_Color32Edit(const char* label, color_t* color);

bool ImGui_EditRect(const char* label, rect_t* p_rect, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

bool ImGui_ToolbarButton(const char* label, bool active, const char* tooltip);

bool ImGui_InputText(const char* label, void** p_str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
               void* user_data = nullptr);

bool ImGui_InputTextMultiline(const char* label, void** p_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0,
                        ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

void get_debug_node_path(entity_t e, char buffer[1024]);

typedef struct text_layer_effect_ text_layer_effect_t;

void guiTextLayerEffect(text_layer_effect_t* layer);

void guiSprite(const sprite_t* sprite);

void guiFont(const font_t* font);
