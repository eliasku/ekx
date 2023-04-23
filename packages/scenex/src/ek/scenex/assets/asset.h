#ifndef SCENEX_C_ASSET_H
#define SCENEX_C_ASSET_H

#include "asset_manager.h"
#include <calo_stream.h>

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
    void (*load)(asset_t* asset);
    void (*unload)(asset_t* asset);
    void (*poll)(asset_t* asset);
    void (*do_load)(asset_t* asset);
    void (*do_unload)(asset_t* asset);
    float (*progress)(asset_t* asset);
    uint32_t type_size;
};

void init_asset_types(void);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_C_ASSET_H
