#pragma once

#include "widgets.h"

#include <ek/scenex/app/base_game.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/interaction_system.h>
#include <ek/canvas.h>

#include <ek/editor/imgui/imgui.hpp>
#include <ekx/app/profiler.h>

static float getterProfilerTrackValue(void* data, int idx) {
    struct profiler_track* track = (struct profiler_track*) data;
    return track->hist[(track->hist_i + idx) % PROFILE_TRACK_CAPACITY];
}

void draw_stats_window(void) {
    const ek_canvas_frame_stats stats = canvas.stats;
    const float drawable_area = ek_app.viewport.width * ek_app.viewport.height;
    ImGui_Text("%ld µs | dc: %u | tri: %u | fill: %d%%",
                (long) (game_app_state.frame_timer.dt * 1000000.0f),
                stats.draw_calls,
                stats.triangles,
                (int) (100.0f * stats.fill_area / drawable_area)
    );
    int entities_count = ecx.size;
    int entities_available = ECX_ENTITIES_MAX_COUNT - entities_count;
    ImGui_Text("%d entities | %d free", entities_count - 1, entities_available);

    auto hitTarget = g_interaction_system.hitTarget_;
    if (is_entity(hitTarget)) {
        char buffer[1024];
        get_debug_node_path(hitTarget, buffer);
        ImGui_Text("Hit Target: 0x%08X %s", hitTarget.id, buffer);
    }

    for (uint32_t i = 0; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        struct profiler_track* track = &s_profile_metrics.tracks[i];
        ImGui_PushID((uintptr_t)track);
        ImGui::PlotLines("", &getterProfilerTrackValue, track, PROFILE_TRACK_CAPACITY);
        ImGui::SameLine();
        ImGui_Text(track->title_format, track->name, (int) track->value);
        ImGui_PopID();
    }
}
