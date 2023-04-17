#include "Widgets.hpp"
#include "../imgui/imgui.hpp"

namespace ImGui {

struct InputTextCallback_UserData {
    ek::String* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

static int InputTextCallback_ek_String(ImGuiInputTextCallbackData* data) {
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*) data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        auto* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->reserve(data->BufTextLen);
        data->Buf = (char*) str->c_str();
    } else if (user_data->ChainCallback) {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool InputText(const char* label, ek::String* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
               void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, str->data(), str->capacity() + 1, flags, InputTextCallback_ek_String, &cb_user_data);
}

bool InputTextMultiline(const char* label, ek::String* str, const ImVec2& size, ImGuiInputTextFlags flags,
                        ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextMultiline(label, str->data(), str->capacity() + 1, size, flags, InputTextCallback_ek_String,
                              &cb_user_data);
}

void HelpMarker(const char* desc) {
    ImGui::TextDisabled(ICON_FA_QUESTION_CIRCLE);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool Color32Edit(const char* label, color_t* color) {
    vec4_t v = vec4_color(*color);
    const bool changed = ImGui::ColorEdit4(label, v.data);
    if (changed) {
        *color = color_vec4(v);
    }
    return changed;
}

bool EditRect(const char* label, float* xywh, float v_speed, const char* format, float power) {
    ImGui::PushID(xywh);
    ImGui::LabelText(label, "x: %0.2f y: %0.2f w: %0.2f h: %0.2f", xywh[0], xywh[1], xywh[2], xywh[3]);
    bool changed = ImGui::DragFloat2("Position", xywh, v_speed, 0, 0, format, power);
    changed = changed || ImGui::DragFloat2("Size", xywh + 2, v_speed, 0, FLT_MAX, format, power);
    ImGui::PopID();
    return changed;
}

bool ToolbarButton(const char* label, bool active, const char* tooltip) {
    PushStyleColor(ImGuiCol_Button, active ? 0xFFFF7700 : 0x11111111);
    PushStyleColor(ImGuiCol_Text, active ? 0xFFFFFFFF : 0xFFCCCCCC);
    bool res = Button(label);
    if (tooltip && IsItemHovered()) {
        SetTooltip("%s", tooltip);
    }
    PopStyleColor(2);
    return res;
}

}

void get_debug_node_path(entity_t e, char buffer[1024]) {
    entity_t entity = e;
    const char* names[32];
    int depth = 0;
    while (entity.id && depth < 32) {
        const node_t* node = Node_get(entity);
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
    static const char* names[4] = {
            "Text",
            "Stroke1",
            "Stroke2",
            "Shadow"
    };
    return names[type];
}

void guiTextLayerEffect(text_layer_effect_t* layer) {
    ImGui::PushID(&layer);

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

        ImGui::Color32Edit("Color", &layer->color);
    }
    ImGui::PopID();
}

void guiSprite(const sprite_t* sprite) {
    if (!sprite) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "ERROR: sprite resource slot could not be null");
        return;
    }
    if (!sprite->image_id || !(sprite->state & SPRITE_LOADED)) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "sprite is not loaded");
        return;
    }
    const auto sprite_image = REF_RESOLVE(res_image, sprite->image_id);
    if (!sprite_image.id) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "sprite image is not available");
        return;
    }
    auto rc = sprite->rect;
    auto uv0 = sprite->tex.position;
    auto uv1 = rect_rb(sprite->tex);
    void* tex_id = (void*) (uintptr_t) sprite_image.id;
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
                ImVec2{uv1.x, uv1.y}
        );
    }
}

static const char* font_type_names[2] = {
        [FONT_TYPE_BITMAP] = "Bitmap Font",
        [FONT_TYPE_TTF] = "TTF Font",
};

void guiFont(const font_t* font) {
    if (!font) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "ERROR: font resource slot could not be null");
        return;
    }
    if (!font->loaded_) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "not loaded");
        return;
    }
    ImGui::Text("Type: %s", font_type_names[font->fontType]);
    ImGui::Text("Glyphs: %u", arr_size(font->map.entries));
}

