#include "node_events.h"
#include "node.h"

ecx_component_type NodeEvents;

static void NodeEvents_ctor(component_handle_t i) {
    node_events_t* p = ((node_events_t*) NodeEvents.data[0]) + i;
    new(p)node_events_();
}

static void NodeEvents_dtor(component_handle_t i) {
    node_events_t* p = ((node_events_t*) NodeEvents.data[0]) + i;
    p->~node_events_();
    //memset(p, 0, sizeof(node_events_t));
}

void setup_node_events(void) {
    init_component_type(&NodeEvents, (ecx_component_type_decl) {
            "NodeEvents", 8, 1, {sizeof(node_events_t)}
    });
    NodeEvents.ctor = NodeEvents_ctor;
    NodeEvents.dtor = NodeEvents_dtor;
}

void add_node_event_listener(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event)) {
    add_node_events(e)->signal.add(event_type, callback);
}

void add_node_event_listener_once(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event)) {
    add_node_events(e)->signal.once(event_type, callback);
}

node_event_t node_event(string_hash_t event_type, entity_t e) {
    node_event_t r;
    r.type = event_type;
    r.source = e;
    r.receiver = e;
    r.data.pointer = NULL;
    r.processed = false;
    return r;
}

bool emit_node_event(entity_t e, const node_event_t* event) {
    EK_ASSERT(is_entity(e));
    node_events_t* eh = get_node_events(e);
    if (eh) {
        node_event_t* mutable_event = (node_event_t*) event;
        mutable_event->receiver = e;
        eh->signal.emit_(event->type, event);
        return true;
    }
    return false;
}

void dispatch_broadcast(entity_t e, const node_event_t* event) {
    emit_node_event(e, event);
    e = get_first_child(e);
    while (e.id) {
        dispatch_broadcast(e, event);
        e = get_next_child(e);
    }
}

void dispatch_bubble(entity_t e, const node_event_t* event) {
    while (e.id) {
        emit_node_event(e, event);
        e = get_parent(e);
    }
}

void broadcast(entity_t e, string_hash_t event_type) {
    node_event_t event = node_event(event_type, e);
    dispatch_broadcast(e, &event);
}

void broadcast_f32(entity_t e, string_hash_t event_type, float value) {
    node_event_t event = node_event(event_type, e);
    event.data.f32 = value;
    dispatch_broadcast(e, &event);
}

void broadcast_i32(entity_t e, string_hash_t event_type, int32_t value) {
    node_event_t event = node_event(event_type, e);
    event.data.i32 = value;
    dispatch_broadcast(e, &event);
}

void broadcast_cstr(entity_t e, string_hash_t event_type, const char* payload) {
    node_event_t event = node_event(event_type, e);
    event.data.cstr = payload;
    dispatch_broadcast(e, &event);
}

void notify_parents(entity_t e, string_hash_t event_type) {
    node_event_t event = node_event(event_type, e);
    event.data.cstr = "";
    dispatch_bubble(e, &event);
}

