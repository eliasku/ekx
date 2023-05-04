#include "profiler.h"

#include <ek/scenex/text/text_engine.h>
#include <ek/canvas.h>

// TODO: EK_DEV_BUILD
#ifndef ENABLE_PROFILER

#if !defined(NDEBUG) || defined(EK_DEV_TOOLS)
#define ENABLE_PROFILER
#endif

#endif

void fps_counter_update(fps_counter* fps_counter, float dt) {
    if (FPS_MEASUREMENTS_PER_SECONDS > 0) {
        // estimate average FPS for some period
        fps_counter->counter += 1.0f;
        fps_counter->accum += dt;
        const float period = 1.0f / FPS_MEASUREMENTS_PER_SECONDS;
        if (fps_counter->accum >= period) {
            fps_counter->average = fps_counter->counter * FPS_MEASUREMENTS_PER_SECONDS;
            fps_counter->accum -= period;
            fps_counter->counter = 0.0f;
        }
    } else {
        fps_counter->average = dt > 0.0f ? (1.0f / dt) : 0.0f;
    }
}

profile_metrics_t s_profile_metrics;

float calculateY(const struct profiler_track* track, float val) {
    return roundf(PROFILE_TRACK_HEIGHT * (val - track->min) / (track->max - track->min));
}

color_t calculateColor(const struct profiler_track* track, float val) {
    return lerp_color(RGB(0x00FF00), RGB(0xFF0000),
                      clamp((val - track->th_min) / (track->th_max - track->th_min), 0, 1));
}

void write_track_value(struct profiler_track* track, float hist_value, float value) {
    track->hist[track->hist_i % 60] = hist_value;
    ++track->hist_i;
    track->value = value;
}


void profiler_init(void) {
#ifdef ENABLE_PROFILER
    struct profiler_track* track;
    for (uint32_t i = 0; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        track = &s_profile_metrics.tracks[i];
        track->name = "-";
        track->title_format = "%s: %d";
        track->th_min = 4;
        track->th_max = 8;
        track->min = 0;
        track->max = 16;
    }

    track = &s_profile_metrics.tracks[PROFILE_FPS];
    track->name = "FPS";
    track->title_format = "%s: %d";
    track->th_min = 1000.0f / 60.0f;
    track->th_max = 1000.0f / 30.0f;
    track->min = 16.0f;
    track->max = 100.0f;

    track = &s_profile_metrics.tracks[PROFILE_TRIANGLES];
    track->name = "TRIS";
    track->title_format = "%s: %d";
    track->th_min = 1000;
    track->th_max = 2000;
    track->min = 0;
    track->max = 3000;

    track = &s_profile_metrics.tracks[PROFILE_DRAW_CALLS];
    track->name = "DC";
    track->title_format = "%s: %d";
    track->th_min = 20;
    track->th_max = 50;
    track->min = 0;
    track->max = 200;

    track = &s_profile_metrics.tracks[PROFILE_FILL_RATE];
    track->name = "FR";
    track->title_format = "%s: %d";
    track->th_min = 1;
    track->th_max = 3;
    track->min = 0;
    track->max = 5;

    s_profile_metrics.tracks[PROFILE_FRAME].name = "FRAME";
    s_profile_metrics.tracks[PROFILE_UPDATE].name = "UPDATE";
    s_profile_metrics.tracks[PROFILE_RENDER].name = "RENDER";
    s_profile_metrics.tracks[PROFILE_OVERLAY].name = "OVERLAY";
    s_profile_metrics.tracks[PROFILE_EVENTS].name = "EVENTS";
    s_profile_metrics.tracks[PROFILE_END].name = "END";
#endif
}

void profiler_add_time(uint32_t metric, float time) {
    EK_ASSERT(metric < PROFILE_TRACKS_MAX_COUNT);
    (void) metric;
    (void) time;
#ifdef ENABLE_PROFILER
    struct profiler_track* track = &s_profile_metrics.tracks[metric];
    track->accum += time;
#endif
}

void profiler_update(float dt) {
#ifdef ENABLE_PROFILER
    fps_counter_update(&s_profile_metrics.fps, dt);
    write_track_value(&s_profile_metrics.tracks[PROFILE_FPS], dt * 1000.0f, s_profile_metrics.fps.average);

    for (uint32_t i = PROFILE_FPS + 1; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        struct profiler_track* track = &s_profile_metrics.tracks[i];
        write_track_value(track, track->accum, track->accum);
        track->accum = 0.0f;
    }
#endif
}


void profiler_render_begin(float drawable_area) {
#ifdef ENABLE_PROFILER
    s_profile_metrics.drawable_area = drawable_area;
    s_profile_metrics.tracks[PROFILE_FILL_RATE].accum = -canvas.stats.fill_area;
    s_profile_metrics.tracks[PROFILE_DRAW_CALLS].accum = -(float) canvas.stats.draw_calls;
    s_profile_metrics.tracks[PROFILE_TRIANGLES].accum = -(float) canvas.stats.triangles;
#endif
}

void profiler_render_end(void) {
#ifdef ENABLE_PROFILER
    s_profile_metrics.tracks[PROFILE_FILL_RATE].accum += canvas.stats.fill_area;
    s_profile_metrics.tracks[PROFILE_FILL_RATE].accum /= s_profile_metrics.drawable_area;
    s_profile_metrics.tracks[PROFILE_DRAW_CALLS].accum += (float) canvas.stats.draw_calls;
    s_profile_metrics.tracks[PROFILE_TRIANGLES].accum += (float) canvas.stats.triangles;
#endif
}

void profiler_draw_text(const struct profiler_track* track) {
    draw_text_format(track->title_format, track->name, (int) track->value);
}

void profiler_draw_graph(const struct profiler_track* track) {
    const int samples = PROFILE_TRACK_CAPACITY;
    float width = (float) PROFILE_TRACK_CAPACITY;
    float x = 0.0f;
    float x0 = 0.0f;
    float prev = calculateY(track, track->hist[track->hist_i % PROFILE_TRACK_CAPACITY]);
    canvas_set_empty_image();
    canvas_quad_color(0, 0, width, PROFILE_TRACK_HEIGHT, ARGB(0x77000000));
    for (int i = 0; i < samples; ++i) {
        float val = track->hist[(track->hist_i + i) % PROFILE_TRACK_CAPACITY];
        float curr = calculateY(track, val);
        if (curr != prev || i + 1 == samples) {
            if (x > x0) {
                canvas_quad_color(x0, PROFILE_TRACK_HEIGHT - prev, x - x0, 1, calculateColor(track, prev));
            }
            if (curr != prev) {
                canvas_quad_color(x, PROFILE_TRACK_HEIGHT - curr, 1, curr - prev, calculateColor(track, curr));
            }
            x0 = x + 1;
        }
        prev = curr;
        x += 1;
    }
}

void profiler_draw(const game_display_info* display_info) {
#ifdef ENABLE_PROFILER
    if (!s_profile_metrics.enabled) {
        return;
    }
    const float scale = display_info->dpi_scale;
    canvas.matrix[0].rot = (mat2_t){{scale, 0, 0, scale}};
    canvas.matrix[0].pos = display_info->insets.xy;
    canvas_set_empty_image();

    canvas_save_matrix();
    for (uint32_t i = 0; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        profiler_draw_graph(s_profile_metrics.tracks + i);
        canvas_translate(vec2(0, 35));
    }
    canvas_restore_matrix();

    canvas_save_matrix();
    text_format_t tf = text_format(PROFILER_FONT, 10);
    tf.leading = 1;
    text_engine.format = tf;
    text_engine.position = vec2(2, 2 + tf.size);
    for (uint32_t i = 0; i < PROFILE_TRACKS_MAX_COUNT; ++i) {
        profiler_draw_text(s_profile_metrics.tracks + i);
        canvas_translate(vec2(0, 35));
    }
    canvas_restore_matrix();
#endif
}

