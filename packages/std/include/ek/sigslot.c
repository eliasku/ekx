#include "sigslot.h"
#include "buf.h"
#include "assert.h"

signal_slot_t* sig_connect(signal_t* signal, signal_callback_t callback, string_hash_t type) {
    EK_ASSERT(signal);
    EK_ASSERT(callback);
    EK_ASSERT(signal->flags == 0 && "signal is const during emit");
    signal_slot_t* connection = (signal_slot_t*)malloc(sizeof(signal_slot_t));
    connection->callback = callback;
    connection->type = type;
    connection->once = false;
    arr_push(signal->slots, connection);
    return connection;
}

void sig_disconnect(signal_t* signal, signal_slot_t* connection) {
    EK_ASSERT(signal);
    EK_ASSERT(connection);
    EK_ASSERT(signal->flags == 0 && "signal is const during emit");
    for(uint32_t i = 0; i < arr_size(signal->slots);) {
        if(signal->slots[i] == connection) {
            arr_erase_at(signal->slots, i);
        }
        else {
            ++i;
        }
    }
    free(connection);
}

void sig_emit(signal_t* signal, string_hash_t type, void* data) {
    EK_ASSERT(signal);
    EK_ASSERT(signal->flags == 0 && "signal nested emit forbidden");
    signal->flags = 1;
    for(uint32_t i = 0; i < arr_size(signal->slots);) {
        signal_slot_t* slot = signal->slots[i];
        if(!slot->type || type == slot->type) {
            slot->callback(slot, data);
            if(slot->once) {
                arr_erase_at(signal->slots, i);
                free(slot);
                continue;
            }
        }
        ++i;
    }
    signal->flags = 0;
}

void sig_close(signal_t* signal) {
    EK_ASSERT(signal);
    EK_ASSERT(signal->flags == 0 && "signal nested emit forbidden");
    arr_for(p_slot, signal->slots) {
        free(*p_slot);
    }
    arr_reset(signal->slots);
}
