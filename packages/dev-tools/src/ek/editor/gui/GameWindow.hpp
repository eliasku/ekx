#pragma once

#include <calo_stream.h>

struct GameWindow {
    bool paused;
    float time_scale;
    bool profiler;
};

void game_window_draw(void);
void game_window_load(calo_reader_t* r);
void game_window_save(calo_writer_t* w);

extern GameWindow editor_game_window;
