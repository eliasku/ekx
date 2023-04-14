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

void sg_load(sg_file_t* out, const void* data, uint32_t size);

const sg_node_data_t* sg_get(const sg_file_t* sg, string_hash_t libraryName);

entity_t sg_create(string_hash_t library, string_hash_t name, entity_t parent);

rect_t sg_get_bounds(string_hash_t library, string_hash_t name);

entity_t create_node2d(string_hash_t tag);
entity_t create_node2d_in(entity_t parent, string_hash_t tag, int index);

struct res_sg {
    string_hash_t names[16];
    sg_file_t data[16];
    rr_man_t rr;
};

extern struct res_sg res_sg;

void setup_res_sg(void);

#define R_SG(name) REF_NAME(res_sg, name)

#ifdef __cplusplus
};
#endif

#endif // SCENE_SG_FACTORY_H
