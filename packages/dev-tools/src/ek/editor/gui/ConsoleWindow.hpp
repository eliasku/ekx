#pragma once

#include "EditorWindow.hpp"
#include <ek/ds/Array.hpp>
#include <ek/ds/PodArray.hpp>
#include <ek/log.h>

struct ConsoleMsg {
    char text[1024];
    const char* icon;
    const char* file;
    int line;
    ImU32 color;
    log_level_t verbosity;
    uint8_t tick;
};

struct VerbosityFilterInfo {
    const char* name;
    const char* icon;
    ImU32 color;
    log_level_t verbosity;
    unsigned count;
    bool show;
};

struct ConsoleWindow : public EditorWindow {

    ConsoleWindow();

    ~ConsoleWindow() override = default;

    void onDraw() override;

    ConsoleMsg messages[1024];
    uint32_t messages_num = 0;
    uint32_t messages_cur = 0;
    PodArray<const char*> commands;
    PodArray<const char*> candidates;
    PodArray<char*> history;
    ImGuiTextFilter text_filter{};
    VerbosityFilterInfo infos[6]{};

    char input[1024]{0};

    // -1: new line, 0..History.Size-1 browsing history
    int history_pos = 0;

    bool auto_scroll = true;
    bool scroll_down_required = false;
};
