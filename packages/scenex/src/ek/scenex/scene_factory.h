#ifndef SCENE_SG_FACTORY_H
#define SCENE_SG_FACTORY_H

#include <ek/math.h>
#include <ek/hash.h>
#include <ek/rr.h>
#include <gen_sg.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

entity_t sg_create(string_hash_t library, string_hash_t name, entity_t parent);

rect_t sg_get_bounds(string_hash_t library, string_hash_t name);

entity_t create_node2d(string_hash_t tag);
entity_t create_node2d_in(entity_t parent, string_hash_t tag, int index);

#ifdef __cplusplus
}
#endif

#endif // SCENE_SG_FACTORY_H
