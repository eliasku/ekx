#ifndef SCE_SG_H
#define SCE_SG_H

#include <ek/math.h>
#include <ek/hash.h>
#include <ek/rr.h>
#include <gen_sg.h>

#ifdef __cplusplus
extern "C" {
#endif

void sg_load(sg_file_t* out, const void* data, uint32_t size);

const sg_node_data_t* sg_get(const sg_file_t* sg, string_hash_t library_name);

struct res_sg {
    string_hash_t names[16];
    sg_file_t data[16];
    rr_man_t rr;
};

extern struct res_sg res_sg;

void setup_res_sg(void);

#define R_SG(name) REF_NAME(res_sg, name)

#ifdef __cplusplus
}
#endif

#endif // SCE_SG_H
