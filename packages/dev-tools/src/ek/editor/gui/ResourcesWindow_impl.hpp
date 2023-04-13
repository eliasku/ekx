#pragma once

#include "ResourcesWindow.hpp"

#include <gen_sg.h>
#include <ek/scenex/scene_factory.h>
#include <ek/scenex/3d/scene3d.h>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/text/font.h>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/scenex/text/font.h>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/BitmapFont.hpp>

namespace ek {

/// assets

void draw_sprite_info(void* asset) {
    guiSprite((const sprite_t*)asset);
}

void draw_font_info(void* asset) {
    guiFont((const font_t*)asset);
}

void draw_sg_info(void* asset) {
    const sg_file_t* sg_file = (const sg_file_t*) asset;
    if(!sg_file) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "null");
        return;
    }
    uint32_t scenes_count = arr_size(sg_file->scenes);
    if (ImGui::TreeNode("##scene-list", "Scenes (%u)", scenes_count)) {
        for(uint32_t i = 0; i < scenes_count; ++i) {
            string_hash_t name = sg_file->scenes[i];
            ImGui::Text("%u %s", name, hsp_get(name));
        }
        ImGui::TreePop();
    }
    uint32_t linkages_count = arr_size(sg_file->linkages);
    if (ImGui::TreeNode("##linkages-list", "Linkages (%u)", linkages_count)) {
        for(uint32_t i = 0; i < linkages_count; ++i) {
            sg_scene_info_t info = sg_file->linkages[i];
            auto* node = sg_get(sg_file, info.linkage);
            if (ImGui::TreeNode(node, "%u %s -> %u %s", info.name, hsp_get(info.name), info.linkage,
                                hsp_get(info.linkage))) {
                if (node) {
                    ImGui::TextUnformatted("todo:");
                } else {
                    ImGui::TextDisabled("Not found");
                };
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void draw_atlas_info(void* asset) {
    atlas_ptr p_atlas = *(atlas_ptr*) asset;
    if(!p_atlas) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "null");
        return;
    }

    auto pagesCount = p_atlas->pages.size();
    ImGui::Text("Page Count: %u", pagesCount);
    static float pageScale = 0.25f;
    ImGui::SliderFloat("Scale", &pageScale, 0.0f, 1.0f);
    for (int i = 0; i < pagesCount; ++i) {
        const sg_image page = REF_RESOLVE(res_image, p_atlas->pages[i]);
        if (page.id) {
            ImGui::Text("Page #%d", i);
            const sg_image_desc info = sg_query_image_desc(page);
            const float width = pageScale * (float) info.width;
            const float height = pageScale * (float) info.height;
            ImGui::Image((void*) (uintptr_t) page.id, ImVec2{width, height});
        } else {
            ImGui::TextDisabled("Page #%d", i);
        }
    }

    for (const auto spr: p_atlas->sprites) {
        const auto* sprite = &REF_RESOLVE(res_sprite, spr);
        ImGui::Text("%s (GID: %u)", hsp_get(res_sprite.names[spr]), spr);
        if (sprite->state & SPRITE_LOADED) {
            guiSprite(sprite);
        } else {
            ImGui::TextDisabled("Unloaded");
        }
        ImGui::Separator();
    }
}

void draw_dynamic_atlas_info(void* asset) {
    dynamic_atlas_ptr p_atlas = *(dynamic_atlas_ptr*) asset;

    if(!p_atlas) {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "null");
        return;
    }
    if(ImGui::Button("Reset")) {
        p_atlas->reset();
    }
    auto pagesCount = p_atlas->pages_.size();
    ImGui::Text("Page Size: %d x %d", p_atlas->pageWidth, p_atlas->pageHeight);
    ImGui::Text("Page Count: %u", pagesCount);
    static float pageScale = 0.25f;
    ImGui::SliderFloat("Scale", &pageScale, 0.0f, 1.0f);
    for (int i = 0; i < pagesCount; ++i) {
        const sg_image page = p_atlas->get_page_image(i);
        if (page.id) {
            ImGui::Text("Page #%d", i);
            const sg_image_desc info = sg_query_image_desc(page);
            const float width = pageScale * (float) info.width;
            const float height = pageScale * (float) info.height;
            ImGui::Image((void*) (uintptr_t) page.id, ImVec2{width, height});
        } else {
            ImGui::TextDisabled("Page #%d", i);
        }
    }
}

void draw_material3d_info(void* asset) {
    material3d_t* m = (material3d_t*) asset;

//    if(!m->is_loaded) {
//        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "null");
//        return;
//    }
    ImGui::ColorEdit3("Ambient", m->ambient.data);
    ImGui::ColorEdit3("Diffuse", m->diffuse.data);
    ImGui::ColorEdit3("Specular", m->specular.data);
    ImGui::ColorEdit3("Emission", m->emission.data);
    ImGui::DragFloat("Shininess", &m->shininess, 0.1f, 1.0f, 128.0f);
    ImGui::DragFloat("Roughness", &m->roughness, 0.01f, 0.001f, 1.0f);
}

void draw_mesh3d_info(void* asset) {
    static_mesh_t* m = (static_mesh_t*) asset;
    ImGui::Text("Indices: %i", m->indices_count);
}

void draw_rr_items(const char* type_name, rr_man_t* rr, void (* fn)(void* item)) {
    char buff[128];
    snprintf(buff, sizeof buff, "%s (%u / %u)###%s", type_name, rr->num, rr->max, type_name);
    if (ImGui::BeginTabItem(buff)) {
        uint8_t* item_data = (uint8_t*) rr->data;
        for (uint32_t i = 0; i < rr->num; ++i) {
            string_hash_t name = rr->names[i];
            ImGui::BeginDisabled(!fn);
            if (ImGui::TreeNode(item_data, "%s (0x%08X)", hsp_get(name), name)) {
                if (fn) {
                    fn(item_data);
                }
                ImGui::TreePop();
            }
            ImGui::EndDisabled();
            item_data += rr->data_size;
        }
        ImGui::EndTabItem();
    }
}

void ResourcesWindow::onDraw() {
    if (ImGui::BeginTabBar("res_by_type", 0)) {
        draw_rr_items("image", &res_image.rr, 0);
        draw_rr_items("shader", &res_shader.rr, 0);
        draw_rr_items("sprite", &res_sprite.rr, draw_sprite_info);
        draw_rr_items("particle", &res_particle.rr, 0);
        draw_rr_items("audio", &res_audio.rr, 0);
        draw_rr_items("font", &res_font.rr, draw_font_info);
        draw_rr_items("sg", &res_sg.rr, draw_sg_info);
        draw_rr_items("atlas", &res_atlas.rr, draw_atlas_info);
        draw_rr_items("dynamic_atlas", &res_dynamic_atlas.rr, draw_dynamic_atlas_info);
        draw_rr_items("material3d", &res_material3d.rr, draw_material3d_info);
        draw_rr_items("mesh3d", &res_mesh3d.rr, draw_mesh3d_info);

        ImGui::EndTabBar();
    }
}

}