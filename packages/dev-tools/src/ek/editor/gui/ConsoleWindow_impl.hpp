#pragma once

// `system` function
#include <cstdlib>
#include <ek/log.h>
#include <ek/print.h>
#include <ek/ds/PodArray.hpp>

typedef struct {
    char text[1024];
    const char* icon;
    const char* file;
    int line;
    ImU32 color;
    log_level_t verbosity;
    uint8_t tick;
} console_msg_t;

typedef struct {
    const char* name;
    const char* icon;
    ImU32 color;
    log_level_t verbosity;
    unsigned count;
    bool show;
} verbosity_filter_t;

static struct {
    console_msg_t messages[1024];
    uint32_t messages_num;
    uint32_t messages_cur;
    PodArray<const char*> commands;
    PodArray<const char*> candidates;
    PodArray<char*> history;
    ImGuiTextFilter text_filter{};
    verbosity_filter_t infos[6];

    char input[1024];

    // -1: new line, 0..History.Size-1 browsing history
    int history_pos;

    bool auto_scroll;
    bool scroll_down_required;
} console;

// string utilities
static void trim_right(char* s) {
    char* it = s + strlen(s);
    while (it > s && it[-1] == ' ') {
        --it;
    }
    *it = '\0';
}

static int on_console_input_command_callback(ImGuiInputTextCallbackData* data) {
    auto& commands = console.commands;
    auto& candidates = console.candidates;
    //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
    switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf) {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            candidates.clear();
            for (auto& command: commands) {
                if (strncasecmp(command, word_start, (int)(word_end - word_start)) == 0) {
                    candidates.push_back(command);
                }
            }

            if (candidates.empty()) {
                // No match
                log_info("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
            } else if (candidates.size() == 1) {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            } else {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for (;;) {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.size() && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0) {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                log_info("Possible matches:");
                for (auto& candidate: candidates) {
                    log_info("- %s", candidate);
                }
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory: {
            // Example of HISTORY
            const int prev_history_pos = console.history_pos;
            if (data->EventKey == ImGuiKey_UpArrow) {
                if (console.history_pos == -1) {
                    console.history_pos = static_cast<int>(console.history.size()) - 1;
                } else if (console.history_pos > 0) {
                    --console.history_pos;
                }
            } else if (data->EventKey == ImGuiKey_DownArrow) {
                if (console.history_pos != -1) {
                    if (++console.history_pos >= console.history.size()) {
                        console.history_pos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != console.history_pos) {
                const char* history_str = (console.history_pos >= 0) ? console.history[console.history_pos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
    }
    return 0;
}

static void clear_console(void) {
    console.messages_num = 0;
    console.messages_cur = 0;
}

static void execute_command(const char* cmd) {
    log_info("$ %s", cmd);

    // Insert into history. First find match and delete it so it can be pushed to the back.
    // This isn't trying to be smart or optimal.
    console.history_pos = -1;
    for (int i = static_cast<int>(console.history.size()) - 1; i >= 0; --i) {
        if (strcasecmp(console.history[i], cmd) == 0) {
            free(console.history[i]);
            console.history.erase_at(i);
            break;
        }
    }
    size_t cmdLen = strlen(cmd);
    char* cmdCopy = (char*)malloc(cmdLen + 1);
    memcpy(cmdCopy, cmd, cmdLen + 1);
    console.history.push_back(cmdCopy);
    // Process command
    if (strcasecmp(cmd, "CLEAR") == 0) {
        clear_console();
    } else if (strcasecmp(cmd, "HELP") == 0) {
        log_info("Commands:");
        for (auto* command: console.commands) {
            log_info("- %s", command);
        }
    } else if (strcasecmp(cmd, "HISTORY") == 0) {
        int first = static_cast<int>(console.history.size()) - 10;
        for (int i = first > 0 ? first : 0; i < console.history.size(); ++i) {
            log_info("%3d: %s", i, console.history[i]);
        }
    } else {
        log_info("Unknown command: %s", cmd);
    }

    // scroll to bottom even if `auto_scroll` is false
    console.scroll_down_required = true;
}

void show_console(void) {
    uint32_t filter_mask = 0;
    char tmpBuffer[64];
    for (int i = 0; i < 5; ++i) {
        verbosity_filter_t* info = &console.infos[i];
        unsigned count = 0;
        for (int j = (int)console.messages_cur - (int)MIN(console.messages_num, 1024); j != console.messages_cur; ++j) {
            auto msg = console.messages[(j < 0 ? (j + 1024) : j) % 1024];
            if (msg.verbosity == info->verbosity) {
                ++count;
            }
        }
        info->count = count;
        snprintf(tmpBuffer, sizeof tmpBuffer, "%s %u###console_verbosity_%i", info->icon, count, i);
        ImGui::PushStyleColor(ImGuiCol_Button, info->show ? 0x44FF7722 : 0x0);
        ImGui::PushStyleColor(ImGuiCol_Text, info->show ? info->color : 0x3FFFFFFF);
        if (ImGui::Button(tmpBuffer)) {
            info->show = !info->show;
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Toggle %s Verbosity", info->name);
        }
        if (info->show) {
            filter_mask |= 1 << i;
        }
        ImGui::SameLine(0, 1);
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    if (ImGui::ToolbarButton(ICON_FA_TRASH, false, "Clear All")) {
        clear_console();
    }
    ImGui::SameLine(0, 0);
    if (ImGui::ToolbarButton(ICON_FA_ANGLE_DOUBLE_DOWN, console.auto_scroll, "Scroll to End")) {
        console.auto_scroll = !console.auto_scroll;
        if (console.auto_scroll) {
            console.scroll_down_required = true;
        }
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::TextUnformatted(ICON_FA_SEARCH);
    ImGui::SameLine();
    console.text_filter.Draw("##logs_filter", 100.0f);
    if (console.text_filter.IsActive()) {
        ImGui::SameLine(0, 0);
        if (ImGui::ToolbarButton(ICON_FA_TIMES_CIRCLE, false, "Clear Filter")) {
            console.text_filter.Clear();
        }
    }

    auto logListSize = ImGui::GetContentRegionAvail();
    logListSize.y -= 30;
    ImGui::BeginChild("log_lines", logListSize);
    for (int j = (int)console.messages_cur - (int)MIN(console.messages_num, 1024); j != console.messages_cur; ++j) {
        console_msg_t* msg = &console.messages[(j < 0 ? (j + 1024) : j) % 1024];
        const char* text = msg->text;
        if (console.text_filter.IsActive() && 
        !console.text_filter.PassFilter(text)) {
            continue;
        }
        if (!!((1 << (int)msg->verbosity) & filter_mask) && console.text_filter.PassFilter(text)) {
            ImGui::PushStyleColor(ImGuiCol_Text, msg->color);
            ImGui::PushID(msg);
            if (ImGui::Selectable(msg->icon)) {
                char buf[512];
                ek_snprintf(buf, sizeof buf, "code --goto %s:%d", msg->file, msg->line);
                log_trace("editor: %s", buf);
                system(buf);
            }
            ImGui::PopID();
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s:%u", msg->file, msg->line);
            }

            ImGui::SameLine(0, 10);
            ImGui::TextUnformatted(text);
        }
    }
    if (console.scroll_down_required || (console.auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
        ImGui::SetScrollHereY(1.0f);
        console.scroll_down_required = false;
    }
    ImGui::EndChild();

    ImGui_Separator();
    int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
    ImGui::TextUnformatted(ICON_FA_TERMINAL);
    ImGui::SameLine();
    bool reclaim_focus = false;
    if (ImGui::InputText("###WindowConsole_InputText", console.input, 1024, flags, on_console_input_command_callback, NULL)) {
        trim_right(console.input);
        if (console.input[0]) {
            execute_command(console.input);
        }
        console.input[0] = '\0';
        reclaim_focus = true;
    }
    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if (reclaim_focus) {
        // Auto focus previous widget
        ImGui::SetKeyboardFocusHere(-1);
    }
}

static void log_message_to_console(log_msg_t msg) {
    if (!msg.text || !msg.text[0]) {
        return;
    }

    const verbosity_filter_t* info = console.infos + MIN(msg.level, 5);

    console_msg_t cmsg = INIT_ZERO;
    cmsg.verbosity = (log_level_t)msg.level;
    cmsg.file = msg.file;
    cmsg.line = msg.line;
    cmsg.tick = (uint8_t)msg.frame;
    cmsg.icon = info->icon;
    cmsg.color = info->color;

    strncat(cmsg.text, msg.text, sizeof cmsg.text);
    console.messages[console.messages_cur++] = cmsg;
    console.messages_cur %= 1024;
    ++console.messages_num;

    if (console.auto_scroll) {
        console.scroll_down_required = true;
    }
}

void init_console(void) {

    log_add_sink(log_message_to_console);

    console.infos[0].verbosity = LOG_LEVEL_TRACE;
    console.infos[0].icon = ICON_FA_GLASSES;
    console.infos[0].name = "Trace";
    console.infos[0].color = 0xFF999999;
    console.infos[0].show = true;

    console.infos[1].verbosity = LOG_LEVEL_DEBUG;
    console.infos[1].icon = ICON_FA_BUG;
    console.infos[1].name = "Debug";
    console.infos[1].color = 0xFFAAAA00;
    console.infos[1].show = true;

    console.infos[2].verbosity = LOG_LEVEL_INFO;
    console.infos[2].icon = ICON_FA_INFO_CIRCLE;
    console.infos[2].name = "Info";
    console.infos[2].color = 0xFFFFFFFF;
    console.infos[2].show = true;

    console.infos[3].verbosity = LOG_LEVEL_WARN;
    console.infos[3].icon = ICON_FA_EXCLAMATION_TRIANGLE;
    console.infos[3].name = "Warning";
    console.infos[3].color = 0xFF44BBFF;
    console.infos[3].show = true;

    console.infos[4].verbosity = LOG_LEVEL_ERROR;
    console.infos[4].icon = ICON_FA_BAN;
    console.infos[4].name = "Error";
    console.infos[4].color = 0xFF4444DD;
    console.infos[4].show = true;

    console.infos[5].verbosity = LOG_LEVEL_FATAL;
    console.infos[5].icon = ICON_FA_BOMB;
    console.infos[5].name = "Fatal";
    console.infos[5].color = 0xFF4444FF;
    console.infos[5].show = true;

    console.commands.push_back("CLEAR");
    console.commands.push_back("HELP");
    console.commands.push_back("HISTORY");
}
