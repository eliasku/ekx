#include "interactive.h"
#include "node_events.h"

ECX_DEFINE_TYPE(interactive_t);

#define Interactive ECX_ID(interactive_t)

void setup_interactive(void) {
    ECX_TYPE(interactive_t, 8);
}

interactive_t* add_interactive(entity_t e) {
    EK_ASSERT(Interactive.index);
    interactive_t* c = (interactive_t*) add_component(&Interactive, e);
    *c = (interactive_t) {0};
    c->bubble = true;
    return c;
}

interactive_t* get_interactive(entity_t e) {
    return (interactive_t*) get_component(&Interactive, e);
}

void clear_all_interactive_events(void) {
    for (uint32_t i = 1; i < Interactive.size; ++i) {
        interactive_t* s = (interactive_t*) Interactive.data[0] + i;
        s->ev_out = false;
        s->ev_over = false;
        s->ev_down = false;
        s->ev_tap = false;
        s->ev_tap_back = false;
    }
}

void interactive_handle(interactive_t* i, entity_t e, string_hash_t type) {
    node_event_t event_obj = node_event(type, e);
    bool should_be_clicked = false;
    if (type == POINTER_EVENT_DOWN) {
        i->pushed = true;
        emit_node_event(e, &event_obj);
        i->ev_down = true;
    } else if (type == POINTER_EVENT_UP) {
        should_be_clicked = i->pushed && i->over;
        i->pushed = false;
        emit_node_event(e, &event_obj);
    } else if (type == POINTER_EVENT_OVER) {
        i->over = true;
        emit_node_event(e, &event_obj);
        i->ev_over = true;
    } else if (type == POINTER_EVENT_OUT) {
        i->over = false;
        i->pushed = false;
        emit_node_event(e, &event_obj);
        i->ev_out = true;
    } else {
        return;
    }

    if (should_be_clicked) {
        i->ev_tap = true;
        event_obj = node_event(POINTER_EVENT_TAP, e);
        emit_node_event(e, &event_obj);
    }
}
