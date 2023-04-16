#ifndef SCENEX_ASSET_MANAGER_H
#define SCENEX_ASSET_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct asset_ asset_t;
typedef asset_t* asset_ptr;

typedef enum {
    ASSET_STATE_INITIAL = 0,
    ASSET_STATE_LOADING = 1,
    ASSET_STATE_READY = 2,
} asset_state_t;

typedef struct {
    const char* base_path;
    asset_ptr* assets;
    float scale_factor;
    uint8_t scale_uid;
} asset_manager_t;

#ifdef __cplusplus
extern "C" {
#endif

extern asset_manager_t asset_manager;

void assets_init(void);

void assets_add(asset_ptr asset);

void assets_load_all(void);

void assets_unload_all(void);

void assets_clear(void);

void assets_set_scale_factor(float scale);

bool assets_is_all_loaded(void);

// TODO: >C
void load_asset(asset_ptr asset);
void unload_asset(asset_ptr asset);
void delete_asset(asset_ptr asset);
asset_state_t get_asset_state(asset_ptr asset);


#ifdef __cplusplus
}
#endif

#endif // SCENEX_ASSET_MANAGER_H
