#pragma once

// TODO: move includes to implementation
#include "gui/GameWindow.hpp"
#include "gui/HierarchyWindow.hpp"
#include "gui/InspectorWindow.hpp"
#include "gui/SceneWindow.hpp"
#include "imgui/ekimgui.h"

typedef struct {
    int dirty;
    int width;
    int height;
    bool hidden;
} editor_config_t;

enum {
    EDITOR_SCENE = 0,
    EDITOR_GAME = 1,
    EDITOR_INSPECTOR = 2,
    EDITOR_HIERARCHY = 3,
    EDITOR_CONSOLE = 4,
    EDITOR_STATS = 5,
    EDITOR_RESOURCES = 6,
    EDITOR_MEMORY = 7,
    EDITOR_WINDOWS_NUM = 8,
};

typedef struct {
    const char* name;
    const char* title;
    void (*draw)(void);
    void (*save)(calo_writer_t* w);
    void (*load)(calo_reader_t* r);
    bool opened;
    bool full_frame;
} editor_wnd_t;

typedef struct {
    editor_config_t config;
    editor_wnd_t windows[EDITOR_WINDOWS_NUM];
} editor_t;

void editor_draw_gui(void);

extern editor_t g_editor;

void show_editor_window(editor_wnd_t* wnd);
void draw_stats_window(void);
void draw_resources_window(void);
void draw_memory_profiler(void);
void init_console(void);
void show_console(void);

