#include <ek/app_native.h>

int ek_app_open_url(const char* url) {
    (void)(url);
    return 1;
}

int ek_app_font_path(char* dest, uint32_t size, const char* font_name) {
    (void)(dest);
    (void)(size);
    (void)(font_name);
    return 1;
}

int ek_app_share(const char* content) {
    (void)(content);
    return 1;
}

int main(int argc, char* argv[]) {
    ek_app__init();
    ek_app.argc = argc;
    ek_app.argv = argv;
    ek_app_main();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        return ek_app.exit_code;
    }
    ek_app__notify_ready();
    while ((ek_app.state & EK_APP_STATE_EXIT_PENDING) == 0) {
    }
    return ek_app.exit_code;
}
