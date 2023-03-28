#include "interactiv.h"
#include "NodeEvents.hpp"

ecx_component_type interactive_type;

void interactive_init(void) {
    init_component_type(&interactive_type, (ecx_component_type_decl) {
            "interactive", 8, 1, {sizeof(interactive_t)}
    });
}

interactive_t* interactive_add(entity_t e) {
    EK_ASSERT(interactive_type.index);
    interactive_t* c = (interactive_t*) add_component(&interactive_type, e);
    *c = (interactive_t) {};
    c->bubble = true;
    return c;
}

interactive_t* interactive_get(entity_t e) {
    return (interactive_t*) get_component(&interactive_type, e);
}

void interactive_clear_all_events(void) {
    for(uint32_t i = 1; i < interactive_type.size; ++i) {
        interactive_t* s = (interactive_t*)interactive_type.data[0] + i;
        s->ev_out = false;
        s->ev_over = false;
        s->ev_down = false;
        s->ev_tap = false;
        s->ev_tap_back = false;
    }
}


void interactive_handle(interactive_t* i, entity_t e, string_hash_t type) {
    using namespace ek;
    NodeEventHandler* eh = ecs::try_get<NodeEventHandler>(e);
    bool shouldBeClicked = false;
    if (type == POINTER_EVENT_DOWN) {
        i->pushed = true;
        if (eh) eh->emit({type, e, {nullptr}, e});
        i->ev_down = true;
    } else if (type == POINTER_EVENT_UP) {
        shouldBeClicked = i->pushed && i->over;
        i->pushed = false;
        if (eh) eh->emit({type, e, {nullptr}, e});
    } else if (type == POINTER_EVENT_OVER) {
        i->over = true;
        if (eh) eh->emit({type, e, {nullptr}, e});

        i->ev_over = true;
    } else if (type == POINTER_EVENT_OUT) {
        i->over = false;
        i->pushed = false;
        if (eh) eh->emit({type, e, {nullptr}, e});
        i->ev_out = true;
    } else {
        return;
    }

    if (shouldBeClicked) {
        i->ev_tap = true;
        if (eh) {
            eh->emit({POINTER_EVENT_TAP, e, {nullptr}, e});
        }
    }
}
