#ifndef EKX_APP_INPUT_STATE_H
#define EKX_APP_INPUT_STATE_H

#include <ek/app.h>
#include <ekx/app/game_display.h>

#ifdef __cplusplus
extern "C" {
#endif

enum touch_state_t {
    TOUCH_STATE_ACTIVE = 1,
    TOUCH_STATE_PRESSED = 2,
    TOUCH_STATE_STARTED = 4,
    TOUCH_STATE_ENDED = 8,
};

enum {
    TOUCH_MAX_COUNT = 8
};

typedef struct touch {
    uint64_t id;
    uint32_t state;
    vec2_t position;
    vec2_t start_position;
    vec2_t end_position;
} touch_t;

enum key_state_t {
    KEY_STATE_ACTIVE = 1,
    KEY_STATE_UP = 2,
    KEY_STATE_DOWN = 4
};

void input_state_process_event(const ek_app_event* event, game_display_info* display_info);

void input_state_post_update(void);

typedef struct input_state {
    uint32_t touches_num;
    touch_t touches[TOUCH_MAX_COUNT];

    uint8_t keys[_EK_KEYCODE_NUM];

    bool hovered_by_editor_gui;
    bool emulate_touch;

    //int keyboard_modifiers_{};
    bool reset_keys;
} input_state_t;

bool is_key(ek_key_code code);

bool is_key_down(ek_key_code code);

bool is_key_up(ek_key_code code);

void reset_keyboard();

touch_t* get_or_create_touch(uint64_t id);

extern input_state_t g_input_state;

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_INPUT_STATE_H
