#include "updater.h"

ECX_DEFINE_TYPE(updater_t);

void setup_updater(void) {
    ECX_TYPE(updater_t, 8);
}

void set_updater(entity_t e, updater_callback_t callback) {
    EK_ASSERT(ECX_ID(updater_t).index);
    updater_t* state = add_component(&ECX_ID(updater_t), e);
    state->callback = callback;
    state->time_layer = 0;
}

void process_updaters(void) {
    for (uint32_t i = 1; i < ECX_ID(updater_t).size; ++i) {
        entity_idx_t ei = ECX_ID(updater_t).handle_to_entity[i];
        updater_t s = ((updater_t*) ECX_ID(updater_t).data[0])[i];
        if (s.callback) {
            s.callback(entity_at(ei), s.time_layer);
        }
    }
}

