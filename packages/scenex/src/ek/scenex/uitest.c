#ifdef EK_UITEST

#include <ekx/app/uitest.h>
#include <ek/print.h>
#include <ek/assert.h>
#include <ek/scenex/app/base_game.h>
#include <ek/log.h>
#include <ekx/app/localization.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/base/node_events.h>
#include <stdlib.h>
#include <stdio.h>

int uitest_step;

typedef struct {
    const char* name;

    void (* run)(void);
} test_reg_t;

static test_reg_t tests[64];
static uint32_t tests_num;

static const char* test_to_run;
static const char* test_screenshots_dir;
static lang_name_t test_lang;

static void set_simulator_display_settings(vec2_t size, vec4_t insets, bool relative, vec2_t applicationBaseSize) {
    if (relative) {
        const float scale = 4.0f;
        const vec2_t aspect = size;
        size.x = applicationBaseSize.x * scale;
        size.y = size.x * aspect.y / aspect.x;
        insets = scale_vec4(insets, applicationBaseSize.x * scale);
    }

    game_app_state.display.simulated = true;
    game_app_state.display.info.size =
    game_app_state.display.info.window = size;
    game_app_state.display.info.dpiScale = 1.0f;
    game_app_state.display.info.insets = insets;
}

static const char* find_argument_value(const char* name, const char* def) {
    if (name && name[0]) {
        const uint32_t n = ek_app.argc;
        for (uint32_t i = 1; i < n; ++i) {
            if (strcmp(ek_app.argv[i - 1], name) == 0) {
                const char* res = ek_app.argv[i];
                if (res) {
                    return res;
                }
            }
        }
    }
    return def;
}

void uitest_screenshot(const char* name) {
    ++uitest_step;
    char path[1024];
    ek_snprintf(path, sizeof path, "%s%i_%s.png", test_screenshots_dir, uitest_step, name);
    game_display_screenshot(&game_app_state.display, path);
}

void uitest_done(void) {
    exit(0);
}

void uitest_fail(void) {
    exit(1);
}

void uitest_click_entity(entity_t e) {
    if (is_entity(e)) {
        button_t* btn = get_button(e);
        if (btn) {
            const node_event_t event = node_event(BUTTON_EVENT_CLICK, e);
            emit_node_event(e, &event);
        }
    }
}

void uitest_start(void) {
    if (test_lang.str[0]) {
        set_language(test_lang);
    }

    if (test_to_run && test_to_run[0]) {
        for (uint32_t i = 0; i < tests_num; ++i) {
            test_reg_t test = tests[i];
            if (strcmp(test.name, test_to_run) == 0) {
                log_info("UI test run: %s", test_to_run);
                if (test.run) {
                    test.run();
                }
                return;
            }
        }
        log_error("UI test %s not found", test_to_run);
        uitest_fail();
    }
}

void uitest(const char* name, void(* run)(void)) {
    tests[tests_num++] = (test_reg_t) {name, run};
}

void uitest_setup(void) {
    test_to_run = find_argument_value("--uitest", "");
    test_lang.str[0] = 0;
    strncat(test_lang.str, find_argument_value("--lang", ""), (sizeof test_lang.str) - 1);
    {
        vec2_t size = vec2(1, 1);
        vec4_t insets = vec4(0, 0, 0, 0);
        int flags = 0;
        sscanf(
                find_argument_value("--display", "9,16,0,0,0,0,1"),
                "%f,%f,%f,%f,%f,%f,%d",
                &size.x, &size.y,
                &insets.x, &insets.y, &insets.z, &insets.w,
                &flags
        );
        const vec2_t base_size = vec2(480, 640);
        set_simulator_display_settings(size, insets, flags == 1, base_size);
    }
    test_screenshots_dir = find_argument_value("--screenshot-output", "");
}

#else

void uitest_setup(void) {}

#endif // EK_UITEST
