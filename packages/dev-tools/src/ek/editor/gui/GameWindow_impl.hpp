#pragma once

#include "../Editor.hpp"

void game_window_draw(void) {
    game_display* display = &game_app_state.display;
    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const ImVec2 displaySize = ImGui::GetContentRegionAvail();
    if (display->color.id && displaySize.x > 0 && displaySize.y > 0) {
        void* texId = (void*)(uintptr_t)display->color.id;

        const float scale = fmin(displaySize.x / display->info.size.x, displaySize.y / display->info.size.y);

        const sg_image_desc info = sg_query_image_desc(display->color);
        const float texCoordX1 = display->info.size.x / (float)info.width;
        const float texCoordY1 = display->info.size.y / (float)info.height;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage(texId, {displayPos.x, displayPos.y},
                           {displayPos.x + scale * display->info.size.x,
                            displayPos.y + scale * display->info.size.y},
                           {0, 0},
                           {texCoordX1, texCoordY1});

        {
            // update size;
            const float k = display->info.dpi_scale;
            display->info.dest_viewport = rect_scale_f(rect(displayPos.x, displayPos.y, displaySize.x, displaySize.y), k);
            display->info.window = vec2(displaySize.x, displaySize.y);
            display->info.size = scale_vec2(vec2(displaySize.x, displaySize.y), k);
        }
    }
    g_input_state.hovered_by_editor_gui = !ImGui::IsWindowHovered(0);
}

void game_window_load(calo_reader_t* r) {
    if (r) {
        g_editor.game.time_scale = read_f32(r);
        g_editor.game.paused = read_u32(r) != 0;
        g_editor.game.profiler = read_u32(r) != 0;
    } else {
        g_editor.game.time_scale = 1.0f;
    }
}

void game_window_save(calo_writer_t* w) {
    write_f32(w, g_editor.game.time_scale);
    write_u32(w, g_editor.game.paused ? 1 : 0);
    write_u32(w, g_editor.game.profiler ? 1 : 0);
}
