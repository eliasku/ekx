#pragma once

#include "Asset.hpp"

#include <ek/log.h>
#include <ek/assert.h>
#include <ek/math.h>
#include <ek/buf.h>

asset_manager_t asset_manager;

void assets_init(void) {
    asset_manager.base_path = "assets";
    asset_manager.scale_factor = 1;
    asset_manager.scale_uid = 1;
}

static uint8_t get_scale_uid(float scale) {
    if (scale > 3.0f) {
        return 4;
    } else if (scale > 2.0f) {
        return 3;
    } else if (scale > 1.0f) {
        return 2;
    }
    return 1;
}

void assets_load_all(void) {
    arr_for (asset, asset_manager.assets) {
        (*asset)->load();
    }
}

void assets_unload_all(void) {
    arr_for (asset, asset_manager.assets) {
        (*asset)->unload();
    }
}

void assets_clear(void) {
    assets_unload_all();

    arr_for (asset, asset_manager.assets) {
        delete (*asset);
    }
    arr_reset((void**)&asset_manager.assets);
}

void assets_set_scale_factor(float scale) {
    auto new_uid = get_scale_uid(scale);
    asset_manager.scale_factor = clamp(scale, 1, 4);
    if (asset_manager.scale_uid != new_uid) {
        log_debug("asset manager: content scale changed to %d%%", (int) (100 * asset_manager.scale_factor));
        asset_manager.scale_uid = new_uid;
        // todo: maybe better naming `update`?
        assets_load_all();
    }
}

void assets_add(asset_ptr asset) {
    EK_ASSERT(asset);
    arr_push(asset_manager.assets, asset);
}

bool assets_is_all_loaded() {
    arr_for (asset, asset_manager.assets) {
        if ((*asset)->state != ASSET_STATE_READY) {
            return false;
        }
    }
    return true;
}
