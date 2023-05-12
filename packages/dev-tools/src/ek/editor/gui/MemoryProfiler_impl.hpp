#pragma once

#include "Widgets.hpp"

#include <ek/core_dbg.h>

void draw_memory_profiler(void);

/// canvas stats
void draw_buffer_chain_stats(const char* name, ek_canvas_buffers* buffers) {
    for (int line = 0; line < 4; ++line) {
        int c = 0;
        for (int i = 0; i < CANVAS_BUFFERS_MAX_COUNT; ++i) {
            if (buffers->lines[line][i].id == 0) {
                i = CANVAS_BUFFERS_MAX_COUNT;
            } else {
                ++c;
            }
        }
        ImGui_Text("%s[%d] count %d", name, line, c);
    }
}

void draw_canvas_stats() {
    ImGui_Text("size: %lu", sizeof canvas);
    ImGui_Text("vb chain size: %lu", sizeof canvas.vbs);
    ImGui_Text("ib chain size: %lu", sizeof canvas.ibs);
    ImGui_Text("vb mem size: %lu", sizeof canvas.vertex);
    ImGui_Text("ib mem size: %lu", sizeof canvas.index);
    draw_buffer_chain_stats("VB", &canvas.vbs);
    draw_buffer_chain_stats("IB", &canvas.ibs);

    ImGui_Separator();

    ImGui_Text("C++ Arrays: %d", ek_core_dbg_get(EK_CORE_DBG_ARRAY));
    ImGui_Text("Pod Arrays: %d", ek_core_dbg_get(EK_CORE_DBG_POD_ARRAY));
    ImGui_Text("Signals: %d", ek_core_dbg_get(EK_CORE_DBG_SIGNAL));
    ImGui_Text("Hashes: %d", ek_core_dbg_get(EK_CORE_DBG_HASH));
    ImGui_Text("Strings: %d", ek_core_dbg_get(EK_CORE_DBG_STRING));

    ImGui_Text("EK_CORE_DBG_INTERACTIVE: %d", ek_core_dbg_get(EK_CORE_DBG_INTERACTIVE));
    ImGui_Text("EK_CORE_DBG_VD: %d", ek_core_dbg_get(EK_CORE_DBG_VD));

    ImGui_Separator();
}


inline double toMB(uint64_t bytes) {
    return static_cast<double>(bytes / 1024) / 1024.0;
}

inline double toKB(uint64_t bytes) {
    return static_cast<double>(bytes) / 1024.0;
}

void drawAllocationsList() {
    // TODO:
    //    AllocationInfo data[10000];
    //    const auto count = allocator.getAllocationsInfo(10000, data);
    //    for (uint32_t i = 0; i < count; ++i) {
    //        auto& info = data[i];
    //        ImGui_Text("%0.2lf %0.2lf", toKB(info.sizeTotal), toKB(info.sizeUsed));
    //        for(unsigned j = 0; j < AllocationInfo::MaxStackDepth; ++j) {
    //            if(info.stack[j]) {
    //                ImGui::BulletText("[%u] %s", j, info.stack[j]);
    //            }
    //        }
    //    }
}

void drawAllocatorMemorySpan() {
    // TODO:
    //
    //    uint64_t MEMORY_PROFILER_BLOCK[4096];
    //    auto count = readAllocationMap(allocator, MEMORY_PROFILER_BLOCK, 4096);
    //    uint32_t i = 0;
    //    uint64_t min = 0xFFFFFFFFFFFFFFFFu;
    //    uint64_t max = 0;
    //    while (i < count) {
    //        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
    //        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
    //        if (pos < min) {
    //            min = pos;
    //        }
    //        if (end > max) {
    //            max = pos;
    //        }
    //    }
    //    i = 0;
    //    constexpr uint32_t MapSize = 1024 * 128;
    //    char BB[MapSize];
    //    memset(BB, '_', MapSize);
    //    uint32_t len = (max - min) / 1024;
    //    if (len > MapSize) {
    //        len = MapSize;
    //    }
    //
    //    while (i < count) {
    //        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
    //        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
    //        uint32_t i0 = (pos - min) / 1024;
    //        uint32_t i1 = (end - min) / 1024;
    //        for (uint32_t x = i0; x < i1 && x < MapSize; ++x) {
    //            BB[x] = '#';
    //        }
    //    }
    //
    //    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    //    i = 0;
    //    const uint32_t NumKilosPerLine = 80;
    //    uint32_t skipped = 0;
    //    while (i < len) {
    //        uint32_t txtLen = 0;
    //        uint32_t occupied = 0;
    //        while (txtLen < NumKilosPerLine && (i + txtLen) < len) {
    //            if (BB[i + txtLen] == '#') {
    //                ++occupied;
    //            }
    //            ++txtLen;
    //        }
    //        if (occupied > 0) {
    //            if (skipped > 0) {
    //                ImGui_Text("-/- Skipped %0.2f MB -/-", (float) (skipped * NumKilosPerLine) / 1024.0f);
    //            }
    //            skipped = 0;
    //            ImGui_TextUnformatted(BB + i, BB + i + txtLen);
    //        } else {
    //            ++skipped;
    //        }
    //        i += txtLen;
    //    }
    //    if (skipped > 0) {
    //        ImGui_Text("-/- Skipped %0.2f MB -/-", (float) (skipped * NumKilosPerLine) / 1024.0f);
    //    }
    //    ImGui::PopFont();
}

void DrawMemoryBlock() {

    // TODO:
    //    auto* stats = allocator.getStats();
    //    if (!stats) {
    //        return;
    //    }
    //
    //    static const char* STATS_MODES[3] = {"Current", "Peak", "All Time"};
    //    if (ImGui::TreeNode("memory_map", "Memory Span: %0.1lf MB", toMB(allocator.getSpanSize()))) {
    //        drawAllocatorMemorySpan(allocator);
    //        ImGui::TreePop();
    //    }
    //    if (ImGui::TreeNode("allocations", "Allocations")) {
    //        drawAllocationsList(allocator);
    //        ImGui::TreePop();
    //    }
    //    if (ImGui::BeginTable("stats", 4)) {
    //        ImGui::TableSetupColumn("Measurement");
    //        ImGui::TableSetupColumn("# Allocations");
    //        ImGui::TableSetupColumn("Effective (MB)");
    //        ImGui::TableSetupColumn("Total (MB)");
    //        ImGui::TableHeadersRow();
    //
    //        for (int i = 0; i < 3; ++i) {
    //            ImGui::TableNextRow();
    //            ImGui::TableSetColumnIndex(0);
    //            ImGui_TextUnformatted(STATS_MODES[i], 0);
    //            ImGui::TableSetColumnIndex(1);
    //            ImGui_Text("%u", stats->allocations[i]);
    //            ImGui::TableSetColumnIndex(2);
    //            ImGui_Text("%0.2lf", toMB(stats->memoryEffective[i]));
    //            ImGui::TableSetColumnIndex(3);
    //            ImGui_Text("%0.2lf", toMB(stats->memoryAllocated[i]));
    //        }
    //        ImGui::EndTable();
    //    }
}

void DrawAllocatorsTree() {
    // TODO:
    //    ImGui::PushID(&allocator);
    //    auto* info = allocator.getStats();
    //    if (info) {
    //        if (ImGui::TreeNode(info->label)) {
    //            DrawMemoryBlock(allocator);
    //            auto* it = info->children;
    //            while (it) {
    //                auto* childInfo = it->getStats();
    //                if (childInfo) {
    //                    DrawAllocatorsTree(*it);
    //                    it = childInfo->next;
    //                } else {
    //                    break;
    //                }
    //            }
    //            ImGui::TreePop();
    //        }
    //    } else {
    //        ImGui_Text("Allocator is not debuggable");
    //    }
    //    ImGui::PopID();
}

void drawECSMemoryStats() {
    ImGui_Text("ECS World Struct Size: %lu", sizeof ecx);
    uint32_t totalUsed = 0;
    uint32_t totalReserved = 0;
    uint32_t totalLookup = 0;
    if (ImGui::BeginTable("components", 6)) {
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Capacity");
        ImGui::TableSetupColumn("Control (KB)");
        ImGui::TableSetupColumn("DATA (KB)");
        ImGui::TableHeadersRow();

        for (uint32_t i = 0; i < ecx.components_num; ++i) {
            auto* header = ecx_components[i];
            if (header) {
                const char* label = header->label;
                const uint32_t cap = arr_capacity(header->handle_to_entity);
                const uint32_t len = header->size;
                uint32_t controlSizeReserved = cap * (sizeof(entity_t) + sizeof(component_type_id));
                uint32_t controlSizeUsed = len * (sizeof(entity_t) + sizeof(component_type_id));
                uint32_t dataSizeReserved = 0;
                uint32_t dataSizeUsed = 0;
                for (uint16_t j = 0; j < header->data_num; ++j) {
                    dataSizeReserved += cap * header->data_stride[j];
                    dataSizeUsed += len * header->data_stride[j];
                }
                uint32_t lookupSize = sizeof(uint16_t) * ECX_ENTITIES_MAX_COUNT;
                //for(uint32_t j = 0; j < ek_sparse_array_get(header->entityToHandle,))
                totalLookup += lookupSize;

                totalUsed += lookupSize + controlSizeUsed + dataSizeUsed;
                totalReserved += lookupSize + controlSizeReserved + dataSizeReserved;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui_Text("%u", i);
                ImGui::TableSetColumnIndex(1);
                ImGui_TextUnformatted(label ? label : "?", 0);

                ImGui::TableSetColumnIndex(2);
                ImGui_Text("%u", len);

                ImGui::TableSetColumnIndex(3);
                ImGui_Text("%u", cap);

                ImGui::TableSetColumnIndex(4);
                ImGui_Text("%0.2lf / %0.2lf", toKB(controlSizeUsed), toKB(controlSizeReserved));

                ImGui::TableSetColumnIndex(5);
                ImGui_Text("%0.2lf / %0.2lf", toKB(dataSizeUsed), toKB(dataSizeReserved));
            }
        }
        ImGui::EndTable();
    }
    ImGui_Text("ECX lookups: %0.2lf KB", toKB(totalLookup));
    ImGui_Text("ECX used: %0.2lf MB", toMB(totalUsed));
    ImGui_Text("ECX reserved: %0.2lf MB", toMB(totalReserved));
}

void draw_memory_profiler(void) {
    draw_canvas_stats();

    if (ImGui::BeginTabBar("Memory Stats")) {
        if (ImGui::BeginTabItem("Allocators")) {
            // TODO:
            ImGui_Text("TODO");
            //DrawAllocatorsTree(memory::systemAllocator);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ECS Memory")) {
            drawECSMemoryStats();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
