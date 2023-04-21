#ifndef SCENEX_NODE_EVENTS_H
#define SCENEX_NODE_EVENTS_H

#include <ecx/ecx.h>

#ifdef __cplusplus

#include <utility>
#include <ek/util/Signal.hpp>
#include <ecx/ecx.hpp>

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    const void* pointer;
    const char* cstr;
    float f32;
    uint32_t u32;
    int64_t i64;
    int i32;
} node_event_data_t;

typedef struct {
    string_hash_t type;
    entity_t source;
    entity_t receiver;
    node_event_data_t data;
    bool processed;
} node_event_t;

typedef struct node_events_ node_events_t;

node_event_t node_event(string_hash_t event_type, entity_t e);

/*** events functions ***/
extern ecx_component_type NodeEvents;
void setup_node_events(void);
#define get_node_events(e) ((node_events_t*)get_component(&NodeEvents, e))
#define add_node_events(e) ((node_events_t*)add_component(&NodeEvents, e))

bool emit_node_event(entity_t e, const node_event_t* event);
void add_node_event_listener(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event));
void add_node_event_listener_once(entity_t e, string_hash_t event_type, void(* callback)(const node_event_t* event));

void dispatch_broadcast(entity_t e, const node_event_t* event);
void dispatch_bubble(entity_t e, const node_event_t* event);

void broadcast(entity_t e, string_hash_t event_type);
void broadcast_f32(entity_t e, string_hash_t event_type, float value);
void broadcast_i32(entity_t e, string_hash_t event_type, int32_t value);
void broadcast_cstr(entity_t e, string_hash_t event_type, const char* payload);

void notify_parents(entity_t e, string_hash_t event_type);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

struct node_events_ {
    ek::Signal<const node_event_t*> signal{};
};

#endif

#endif // SCENEX_NODE_EVENTS_H
