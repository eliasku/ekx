#ifndef EK_LOG_H
#define EK_LOG_H

#include <ek/pre.h>

// inspired by simple API from https://github.com/rxi/log.c

#ifndef __printflike
#define __printflike(fmtarg, firstvararg)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum log_level_t {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_WARN = 3,
    LOG_LEVEL_ERROR = 4,
    LOG_LEVEL_FATAL = 5
} log_level_t;

typedef struct log_msg_t {
    const char* text;
    const char* file;
    int line;
    uint16_t level;
    uint16_t frame;
} log_msg_t;

typedef void (*log_callback_t)(log_msg_t msg);

void log_init(void);

void log_print(log_level_t level, const char* file, int line, const char* fmt, ...) __printflike(4, 5);

void log_tick(void);

void log_add_sink(log_callback_t cb);

void log_set_levels(uint16_t mask);

#ifdef __cplusplus
}
#endif

#ifdef EK_LOG_DEBUG_OUTPUT

#define log_trace(...) log_print(LOG_LEVEL_TRACE, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)
#define log_debug(...) log_print(LOG_LEVEL_DEBUG, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)

#else

#define log_trace(...) ((void)0)
#define log_debug(...) ((void)0)

#endif

#define log_info(...) log_print(LOG_LEVEL_INFO, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)
#define log_warn(...) log_print(LOG_LEVEL_WARN, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)
#define log_error(...) log_print(LOG_LEVEL_ERROR, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)
#define log_fatal(...) log_print(LOG_LEVEL_FATAL, EK_CURRENT_FILE, EK_CURRENT_LINE, __VA_ARGS__)

#ifdef LOG_ALLOCATION

inline static void ek_profile_alloc(const char* label, uint32_t size, const char* file, int line) {
    log_print(LOG_LEVEL_WARN, file, line, "allocate %u (%s)", size, label);
}

#define EK_PROFILE_ALLOC(label, sz) ek_profile_alloc((label), (sz), EK_CURRENT_FILE, EK_CURRENT_LINE)

#else

#define EK_PROFILE_ALLOC(label, sz) ((void)0)

#endif

#endif //EK_LOG_H
