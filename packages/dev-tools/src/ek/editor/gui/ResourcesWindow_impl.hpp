#pragma once

#include <ek/audio.h>
#include <sce/atlas.h>
#include <sce/dynamic_atlas.h>
#include <sce/sg.h>
#include <sce/font.h>
#include <gen_sg.h>

/// assets

void draw_font_info(void* asset) {
    gui_font((const font_t*)asset);
}

void draw_sg_info(void* asset) {
    const sg_file_t* sg_file = (const sg_file_t*)asset;
    if (!sg_file) {
        ImGui_TextError("null");
        return;
    }
    uint32_t scenes_count = arr_size(sg_file->scenes);
    if (ImGui_TreeNode("##scene-list", "Scenes (%u)", scenes_count)) {
        for (uint32_t i = 0; i < scenes_count; ++i) {
            string_hash_t name = sg_file->scenes[i];
            ImGui_Text("%u %s", name, hsp_get(name));
        }
        ImGui_TreePop();
    }
    uint32_t linkages_count = arr_size(sg_file->linkages);
    if (ImGui_TreeNode("##linkages-list", "Linkages (%u)", linkages_count)) {
        for (uint32_t i = 0; i < linkages_count; ++i) {
            sg_scene_info_t info = sg_file->linkages[i];
            const sg_node_data_t* node = sg_get(sg_file, info.linkage);
            if (ImGui_TreeNode(node, "%u %s -> %u %s", info.name, hsp_get(info.name), info.linkage,
                               hsp_get(info.linkage))) {
                if (node) {
                    ImGui_TextUnformatted("todo:", 0);
                } else {
                    ImGui_TextDisabled("Not found");
                };
                ImGui_TreePop();
            }
        }
        ImGui_TreePop();
    }
}

void draw_image_info(void* asset) {
    sg_image* image = (sg_image*)asset;
    if (image->id) {
        static float image_scale = 0.25f;
        ImGui::SliderFloat("Scale", &image_scale, 0.0f, 1.0f);
        const sg_image_desc info = sg_query_image_desc(*image);
        ImGui_Text("%dx%d", info.width, info.height);
        const float width = image_scale * (float)info.width;
        const float height = image_scale * (float)info.height;
        ImGui::Image((void*)(uintptr_t)image->id, ImVec2{width, height});
    } else {
        ImGui_TextDisabled("none");
    }
}

void draw_atlas_info(void* asset) {
    atlas_t* atlas = (atlas_t*)asset;
    uint32_t pages_count = arr_size(atlas->pages);
    ImGui_Text("Page Count: %u", pages_count);
    if (pages_count) {
        static float page_scale = 0.25f;
        ImGui::SliderFloat("Scale", &page_scale, 0.0f, 1.0f);
        for (int i = 0; i < pages_count; ++i) {
            const sg_image page = REF_RESOLVE(res_image, atlas->pages[i]);
            if (page.id) {
                ImGui_Text("Page #%d", i);
                const sg_image_desc info = sg_query_image_desc(page);
                const float width = page_scale * (float)info.width;
                const float height = page_scale * (float)info.height;
                ImGui::Image((void*)(uintptr_t)page.id, ImVec2{width, height});
            } else {
                ImGui_TextDisabled("Page #%d", i);
            }
        }
    }

    arr_for(p_spr, atlas->sprites) {
        R(sprite_t) spr = *p_spr;
        const sprite_t* sprite = &REF_RESOLVE(res_sprite, spr);
        ImGui_Text("%s (GID: %u)", hsp_get(res_sprite.names[spr]), spr);
        if (sprite->state & SPRITE_LOADED) {
            gui_sprite(sprite);
        } else {
            ImGui_TextDisabled("Unloaded");
        }
        ImGui_Separator();
    }
}

void draw_dynamic_atlas_info(void* asset) {
    dynamic_atlas_t* atlas = (dynamic_atlas_t*)asset;

    if (!atlas) {
        ImGui_TextError("null");
        return;
    }
    uint32_t pages_count = arr_size(atlas->pages_);
    ImGui_Text("Page Size: %d x %d", atlas->pageWidth, atlas->pageHeight);
    ImGui_Text("Page Count: %u", pages_count);
    if(pages_count) {
        static float page_scale = 0.25f;
        ImGui::SliderFloat("Scale", &page_scale, 0.0f, 1.0f);
        for (int i = 0; i < pages_count; ++i) {
            const sg_image page = atlas->pages_[i].image;
            if (page.id) {
                ImGui_Text("Page #%d", i);
                const sg_image_desc info = sg_query_image_desc(page);
                const float width = page_scale * (float)info.width;
                const float height = page_scale * (float)info.height;
                ImGui::Image((void*)(uintptr_t)page.id, ImVec2{width, height});
            } else {
                ImGui_TextDisabled("Page #%d : no image", i);
            }
        }
    }
}

// void draw_material3d_info(void* asset) {
//     material3d_t* m = (material3d_t*)asset;
//     ImGui::ColorEdit3("Ambient", m->ambient.data);
//     ImGui::ColorEdit3("Diffuse", m->diffuse.data);
//     ImGui::ColorEdit3("Specular", m->specular.data);
//     ImGui::ColorEdit3("Emission", m->emission.data);
//     ImGui::DragFloat("Shininess", &m->shininess, 0.1f, 1.0f, 128.0f);
//     ImGui::DragFloat("Roughness", &m->roughness, 0.01f, 0.001f, 1.0f);
// }

// void draw_mesh3d_info(void* asset) {
//     static_mesh_t* m = (static_mesh_t*)asset;
//     ImGui_Text("Indices: %i", m->indices_count);
// }

void draw_rr_items(const char* type_name, rr_man_t* rr, void (*fn)(void* item)) {
    char buff[128];
    snprintf(buff, sizeof buff, "%s (%u / %u)###%s", type_name, rr->num, rr->max, type_name);
    if (ImGui_BeginTabItem(buff, NULL, 0)) {
        uint8_t* item_data = (uint8_t*)rr->data;
        item_data += rr->data_size;
        for (uint32_t i = 1; i < rr->num; ++i) {
            string_hash_t name = rr->names[i];
            ImGui_BeginDisabled(!fn);
            if (ImGui_TreeNode(item_data, "[%d] %s (0x%08X)", i, hsp_get(name), name)) {
                if (fn) {
                    fn(item_data);
                }
                ImGui_TreePop();
            }
            ImGui_EndDisabled();
            item_data += rr->data_size;
        }
        ImGui_EndTabItem();
    }
}

void draw_resources_window(void) {
    if (ImGui_BeginTabBar("res_by_type", 0)) {
        draw_rr_items("image", &res_image.rr, draw_image_info);
        draw_rr_items("shader", &res_shader.rr, 0);
        draw_rr_items("sprite", &res_sprite.rr, (void (*)(void*))gui_sprite);
        draw_rr_items("particle", &res_particle.rr, 0);
        draw_rr_items("audio", &res_audio.rr, 0);
        draw_rr_items("font", &res_font.rr, draw_font_info);
        draw_rr_items("sg", &res_sg.rr, draw_sg_info);
        draw_rr_items("atlas", &res_atlas.rr, draw_atlas_info);
        draw_rr_items("dynamic_atlas", &res_dynamic_atlas.rr, draw_dynamic_atlas_info);
        //draw_rr_items("material3d", &res_material3d.rr, draw_material3d_info);
        //draw_rr_items("mesh3d", &res_mesh3d.rr, draw_mesh3d_info);

        ImGui_EndTabBar();
    }
}
