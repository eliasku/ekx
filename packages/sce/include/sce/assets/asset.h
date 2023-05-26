#ifndef SCE_ASSET_H
#define SCE_ASSET_H

#include "asset_manager.h"
#include <calo_stream.h>
#include <ek/hash.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct asset_type_ asset_type;

typedef struct asset_ {
    asset_type* type;
    asset_state_t state;
    int error;
    float weight;
} asset_t;

struct asset_type_ {
    void (*read)(asset_t* asset, calo_reader_t* r);
    void (*load)(asset_t* asset);
    void (*unload)(asset_t* asset);
    void (*poll)(asset_t* asset);
    void (*do_load)(asset_t* asset);
    void (*do_unload)(asset_t* asset);
    float (*progress)(asset_t* asset);
    uint32_t type_size;
    string_hash_t type_name;
};

void init_asset_types(void);

#define NEW_ASSET_TYPE(T) new_asset_type(H(#T),sizeof(T##_asset_t))

asset_type new_asset_type(string_hash_t type_name, uint32_t type_size);
void add_asset_type(asset_type type);

#ifdef __cplusplus
}
#endif

#endif // SCE_ASSET_H
