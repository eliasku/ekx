#ifndef EK_SIGSLOT_H
#define EK_SIGSLOT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct signal_slot_;

typedef void(* signal_callback_t)(struct signal_slot_* slot, void* event);

typedef uint32_t signal_type_t;

typedef struct signal_slot_ {
    signal_callback_t callback;
    signal_type_t type;
    bool once;
    union {
        void* ptr;
        uint64_t u64;
        int64_t i64;
        uint32_t u32;
    } context;
} signal_slot_t;

typedef struct {
    signal_slot_t** slots;
    uint32_t flags;
} signal_t;

signal_slot_t* sig_connect(signal_t* signal, signal_callback_t callback, signal_type_t type);

void sig_disconnect(signal_t* signal, signal_slot_t* connection);

void sig_emit(signal_t* signal, signal_type_t type, void* data);

void sig_close(signal_t* signal);

#ifdef __cplusplus
}
#endif

#endif // EK_SIGSLOT_H
