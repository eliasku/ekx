#include "../imgui/imgui.hpp"
#include "widgets.h"
#include <sce/sprite.h>
#include <sce/font.h>

struct InputTextCallback_UserData {
    void** Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

static int InputTextCallback_ek_String(ImGuiInputTextCallbackData* data) {
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        void** p_str = user_data->Str;
        IM_ASSERT(data->Buf == str_get(*p_str));
        if (arr_capacity(*p_str) < data->BufTextLen) {
            arr_grow(p_str, data->BufTextLen, 1);
        }
        data->Buf = (char*)str_get(*p_str);
    } else if (user_data->ChainCallback) {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui_InputText(const char* label, void** p_str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userdata) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallback_UserData cb_userdata;
    cb_userdata.Str = p_str;
    cb_userdata.ChainCallback = callback;
    cb_userdata.ChainCallbackUserData = userdata;
    return ImGui::InputText(label, (char*)str_get(*p_str), arr_capacity(*p_str) + 1, flags, InputTextCallback_ek_String, &cb_userdata);
}

bool ImGui_InputTextMultiline(const char* label, void** p_str, vec2_t size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userdata) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallback_UserData cb_userdata;
    cb_userdata.Str = p_str;
    cb_userdata.ChainCallback = callback;
    cb_userdata.ChainCallbackUserData = userdata;
    return ImGui::InputTextMultiline(label, (char*)str_get(*p_str), arr_capacity(*p_str) + 1, *(ImVec2*)&size, flags, InputTextCallback_ek_String, &cb_userdata);
}

void ImGui_HelpMarker(const char* desc) {
    ImGui_TextDisabled(ICON_FA_QUESTION_CIRCLE);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui_TextUnformatted(desc, 0);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool ImGui_Color32Edit(const char* label, color_t* color) {
    vec4_t v = vec4_color(*color);
    const bool changed = ImGui::ColorEdit4(label, v.data);
    if (changed) {
        *color = color_vec4(v);
    }
    return changed;
}

bool ImGui_EditRect(const char* label, rect_t* p_rect, float v_speed, const char* format, float power) {
    ImGui_PushID((uintptr_t)p_rect);
    ImGui::LabelText(label, "x: %0.2f y: %0.2f w: %0.2f h: %0.2f", p_rect->x, p_rect->y, p_rect->w, p_rect->h);
    bool changed = ImGui::DragFloat2("Position", &p_rect->x, v_speed, 0, 0, format, power);
    changed = changed || ImGui::DragFloat2("Size", &p_rect->w, v_speed, 0, FLT_MAX, format, power);
    ImGui_PopID();
    return changed;
}

bool ImGui_ToolbarButton(const char* label, bool active, const char* tooltip) {
    ImGui::PushStyleColor(ImGuiCol_Button, active ? 0xFFFF7700 : 0x11111111);
    ImGui::PushStyleColor(ImGuiCol_Text, active ? 0xFFFFFFFF : 0xFFCCCCCC);
    bool res = ImGui_Button(label);
    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    ImGui::PopStyleColor(2);
    return res;
}

void get_debug_node_path(entity_t e, char buffer[1024]) {
    entity_t entity = e;
    const char* names[32];
    int depth = 0;
    while (entity.id && depth < 32) {
        const node_t* node = get_node(entity);
        const string_hash_t tag = node->tag;
        names[depth++] = tag ? hsp_get(tag) : "_";
        entity = node->parent;
    }
    uint32_t len = 0;
    while (depth-- > 0) {
        buffer[len++] = '/';
        const char* str = names[depth];
        uint32_t size = strlen(str);
        if (size > 0) {
            memcpy(buffer + len, str, size);
            len += size;
        }
    }
    buffer[len] = '\0';
}

static const char* get_text_layer_type_name(text_layer_type type) {
    const char* names[4] = {
        "Text",
        "Stroke1",
        "Stroke2",
        "Shadow"};
    return names[type];
}

void gui_text_layer_effect(text_layer_effect_t* layer) {
    ImGui_PushID((uintptr_t)layer);

    if (ImGui::CollapsingHeader(get_text_layer_type_name(layer->type))) {
        ImGui::Checkbox("Visible", &layer->visible);
        ImGui::Checkbox("Show Glyph Bounds", &layer->showGlyphBounds);
        ImGui::DragFloat("Radius", &layer->blurRadius, 1, 0, 8);
        int iterations = layer->blurIterations;
        int strength = layer->strength;
        ImGui::DragInt("Iterations", &iterations, 1, 0, 3);
        ImGui::DragInt("Strength", &strength, 1, 0, 7);
        layer->blurIterations = iterations;
        layer->strength = strength;

        ImGui::DragFloat2("Offset", layer->offset.data, 1, 0, 8);

        ImGui_Color32Edit("Color", &layer->color);
    }
    ImGui_PopID();
}

void gui_sprite(const sprite_t* sprite) {
    if (!sprite) {
        ImGui_TextError("ERROR: sprite resource slot could not be null");
        return;
    }
    if (!sprite->image_id || !(sprite->state & SPRITE_LOADED)) {
        ImGui_TextError("sprite is not loaded");
        return;
    }
    const auto sprite_image = REF_RESOLVE(res_image, sprite->image_id);
    if (!sprite_image.id) {
        ImGui_TextError("sprite image is not available");
        return;
    }
    auto rc = sprite->rect;
    auto uv0 = sprite->tex.position;
    auto uv1 = rect_rb(sprite->tex);
    void* tex_id = (void*)(uintptr_t)sprite_image.id;
    if (sprite->state & SPRITE_ROTATED) {
        ImGui::BeginChild("s", ImVec2{rc.w, rc.h});
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        auto pos = ImGui::GetCursorScreenPos();
        draw_list->AddImageQuad(tex_id,
                                pos,
                                ImVec2{pos.x + rc.w, pos.y},
                                ImVec2{pos.x + rc.w, pos.y + rc.h},
                                ImVec2{pos.x, pos.y + rc.h},
                                ImVec2{uv0.x, uv1.y},
                                ImVec2{uv0.x, uv0.y},
                                ImVec2{uv1.x, uv0.y},
                                ImVec2{uv1.x, uv1.y},
                                IM_COL32_WHITE);
        ImGui::EndChild();
    } else {
        ImGui::Image(
            tex_id,
            ImVec2{rc.w, rc.h},
            ImVec2{uv0.x, uv0.y},
            ImVec2{uv1.x, uv1.y});
    }
}

void gui_font(const font_t* font) {
    if (!font) {
        ImGui_TextError("ERROR: font resource slot could not be null");
        return;
    }
    if (!font->loaded) {
        ImGui_TextError("not loaded");
        return;
    }
    const char* font_type_names[2] = {
        [FONT_TYPE_BITMAP] = "Bitmap Font",
        [FONT_TYPE_TTF] = "TTF Font",
    };
    ImGui_Text("Type: %s", font_type_names[font->font_type]);
    ImGui_Text("Glyphs: %u", arr_size(font->map.entries));
}
