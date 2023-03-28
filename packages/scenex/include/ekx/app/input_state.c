#include "input_state.h"
#include "game_display.h"

input_state_t g_input_state;

touch_t* get_or_create_touch(uint64_t id) {
    touch_t* touches = g_input_state.touches;
    for (uint32_t i = 0; i < g_input_state.touches_num; ++i) {
        if (touches[i].id == id) {
            return &touches[i];
        }
    }
    if(g_input_state.touches_num < TOUCH_MAX_COUNT) {
        touch_t* t = &touches[g_input_state.touches_num++];
        t->id = id;
        return t;
    }
    return NULL;
}

vec2_t coord_screen_to_game_display(const game_display_info* display_info, const vec2_t screenPos) {
    const vec2_t size = display_info->destinationViewport.size;
    const vec2_t offset = display_info->destinationViewport.position;
    const vec2_t displaySize = display_info->size;
    const float invScale = 1.0f / fminf(size.x / displaySize.x, size.y / displaySize.y);
    return scale_vec2(sub_vec2(screenPos, offset), invScale);
}

void emulate_mouse_as_touch(const ek_app_event* event, touch_t* data, vec2_t pos) {
    const bool active_prev = data->state & TOUCH_STATE_ACTIVE;
    // keep prev state
    uint32_t new_state = data->state & TOUCH_STATE_ACTIVE;
    if (!active_prev && event->type == EK_APP_EVENT_MOUSE_DOWN) {
        new_state |= TOUCH_STATE_ACTIVE;
    } else if (active_prev && (event->type == EK_APP_EVENT_MOUSE_UP || event->type == EK_APP_EVENT_MOUSE_EXIT)) {
        // next state is not active
        new_state ^= TOUCH_STATE_ACTIVE;
    }

    const bool active = new_state & TOUCH_STATE_ACTIVE;
    data->position = pos;
    if(active) {
        new_state |= TOUCH_STATE_PRESSED;
        if(!active_prev) {
            new_state |= TOUCH_STATE_STARTED;
            data->start_position = data->position;
        }
    }
    else if(active_prev) {
        new_state |= TOUCH_STATE_ENDED;
        data->end_position = data->position;
    }
    data->state = new_state;
}

void update_touch(const ek_app_event* event, touch_t* data, vec2_t pos) {
    bool active_prev = data->state & TOUCH_STATE_ACTIVE;
    bool active = event->type != EK_APP_EVENT_TOUCH_END;
    uint32_t new_state = 0;
    if(active) {
        new_state |= TOUCH_STATE_ACTIVE;
    }

    data->position = pos;
    if(active) {
        new_state |= TOUCH_STATE_PRESSED;
        if(!active_prev){
            new_state |= TOUCH_STATE_STARTED;
            data->start_position = data->position;
        }
    }
    else if(active_prev) {
        new_state |= TOUCH_STATE_ENDED;
        data->end_position = data->position;
    }
    data->state = new_state;
}

void interaction_system_handle_touch(const ek_app_event* event, vec2_t position);
void interaction_system_handle_mouse(const ek_app_event* event, vec2_t position);
void interaction_system_handle_pause(void);
void interaction_system_send_back_button(void);

void input_state_process_event(const ek_app_event* event, game_display_info* display_info) {
    switch (event->type) {
        case EK_APP_EVENT_TOUCH_START:
        case EK_APP_EVENT_TOUCH_MOVE:
        case EK_APP_EVENT_TOUCH_END: {
            const vec2_t pos = coord_screen_to_game_display(display_info, vec2(event->touch.x, event->touch.y));
            if (!g_input_state.hovered_by_editor_gui) {
                interaction_system_handle_touch(event, pos);
            }
            update_touch(event, get_or_create_touch(event->touch.id), pos);
        }
            break;
        case EK_APP_EVENT_MOUSE_DOWN:
        case EK_APP_EVENT_MOUSE_UP:
        case EK_APP_EVENT_MOUSE_MOVE:
        case EK_APP_EVENT_MOUSE_ENTER:
        case EK_APP_EVENT_MOUSE_EXIT: {
            const vec2_t pos = coord_screen_to_game_display(display_info, vec2(event->mouse.x, event->mouse.y));
            if (!g_input_state.hovered_by_editor_gui) {
                interaction_system_handle_mouse(event, pos);
            }
            if (g_input_state.emulate_touch) {
                emulate_mouse_as_touch(event, get_or_create_touch(1u), pos);
            }
        }
            break;
        case EK_APP_EVENT_WHEEL:
            break;
        case EK_APP_EVENT_KEY_UP:
        case EK_APP_EVENT_KEY_DOWN:
            if (event->type == EK_APP_EVENT_KEY_DOWN && event->key.code == EK_KEYCODE_ESCAPE) {
                interaction_system_send_back_button();
            }
            if (event->key.code != EK_KEYCODE_UNKNOWN) {
                uint8_t state = g_input_state.keys[(uint32_t)event->key.code];
                if (event->type == EK_APP_EVENT_KEY_DOWN) {
                    bool pressed = state & KEY_STATE_ACTIVE;
                    uint8_t new_state = KEY_STATE_ACTIVE;
                    //key.state = true;//!keyboard_modifiers.ctrlKey;
                    if(!pressed) {
                        new_state |= KEY_STATE_DOWN;
                    }
                    g_input_state.keys[(uint32_t)event->key.code] = new_state;
                } else if (event->type == EK_APP_EVENT_KEY_UP) {
                    g_input_state.keys[(uint32_t)event->key.code] = KEY_STATE_UP;
                    //modifiers = none;
                }
            }
            break;
        case EK_APP_EVENT_BACK_BUTTON:
            interaction_system_send_back_button();
            break;
        case EK_APP_EVENT_PAUSE:
            interaction_system_send_back_button();
            interaction_system_handle_pause();
            break;
        default:
            break;
    }
}

bool is_key(ek_key_code code) {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return g_input_state.keys[code] & KEY_STATE_ACTIVE;
    }
    return false;
}

bool is_key_down(ek_key_code code) {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return g_input_state.keys[code] & KEY_STATE_DOWN;
    }
    return false;
}

bool is_key_up(ek_key_code code) {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return g_input_state.keys[code] & KEY_STATE_UP;
    }
    return false;
}

void input_state_post_update() {
    // update keyboard state
    if (g_input_state.reset_keys) {
        memset(g_input_state.keys, 0, sizeof(g_input_state.keys));
        g_input_state.reset_keys = false;
    }
    else {
        for (uint32_t i = 0; i < _EK_KEYCODE_NUM; ++i) {
            // clear up and down triggers
            g_input_state.keys[i] &= KEY_STATE_ACTIVE;
        }
    }

    // update touches info
    unsigned i = 0;
    while (i < g_input_state.touches_num) {
        if (g_input_state.touches[i].state & TOUCH_STATE_ACTIVE) {
            ++i;
        } else {
            --g_input_state.touches_num;
            if(g_input_state.touches_num != i) {
                g_input_state.touches[i] = g_input_state.touches[g_input_state.touches_num];
            }
            g_input_state.touches[g_input_state.touches_num] = (touch_t){0};
        }
    }
}

void reset_keyboard() {
    g_input_state.reset_keys = true;
}

