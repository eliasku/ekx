#include "node_events.h"
#include "node.h"

bool has_node_events(entity_t e) {
    return ecs::has<node_events_t>(e);
}

void add_node_event_listener(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event)) {
    ecs::add<node_events_t>(e)->signal.add(callback, event_type);
}

void add_node_event_listener_once(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event)) {
    ecs::add<node_events_t>(e)->signal.once(callback, event_type);
}

node_event_t node_event(string_hash_t event_type, entity_t e) {
    return (node_event_t) {event_type, e, e, (node_event_data_t) {0}, false};
}

bool emit_node_event(entity_t e, const node_event_t* event) {
    EK_ASSERT(is_entity(e));
    node_events_t* eh = ecs::try_get<node_events_t>(e);
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

