#ifndef SCENEX_NODE_H
#define SCENEX_NODE_H

#include <ecx/ecx.hpp>
#include <ek/assert.h>
#include <ek/hash.h>
#include <ek/ds/PodArray.hpp>
#include "node_api.h"

namespace ek {

struct Node {

    entity_t parent = NULL_ENTITY;
    entity_t child_first = NULL_ENTITY;
    entity_t sibling_next = NULL_ENTITY;
    entity_t child_last = NULL_ENTITY;
    entity_t sibling_prev = NULL_ENTITY;

    uint32_t flags = 0;
    string_hash_t tag = 0;
};

}

typedef ek::Node node_t;

#endif // SCENEX_NODE_H