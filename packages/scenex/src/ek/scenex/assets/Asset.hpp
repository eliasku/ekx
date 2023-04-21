#ifndef SCENEX_ASSET_H
#define SCENEX_ASSET_H

#include <ek/ds/PodArray.hpp>
#include <ek/ds/String.hpp>
#include "asset_manager.h"

struct asset_ {
    asset_() = default;

    virtual ~asset_() = default;

    virtual void load() {
        if (state == ASSET_STATE_INITIAL) {
            state = ASSET_STATE_LOADING;
            this->do_load();
        }
    }

    virtual void unload() {
        if (state == ASSET_STATE_READY) {
            this->do_unload();
            state = ASSET_STATE_INITIAL;
        }
    }

    virtual void poll() {}

    virtual void do_load() {}

    virtual void do_unload() {}

    [[nodiscard]]
    virtual float getProgress() const {
        return state == ASSET_STATE_READY ? 1.0f : 0.0f;
    }

    asset_state_t state = ASSET_STATE_INITIAL;
    int error = 0;
    float weight = 1.0f;
};

struct PackAsset : public asset_ {
    explicit PackAsset(ek::String name);
    void do_load() override;
    void do_unload() override;

    void poll() override;
    [[nodiscard]] float getProgress() const override;

    ek::String name_;
    ek::PodArray<asset_ptr> assets;
    unsigned assetsLoaded = 0;
    bool assetListLoaded = false;
};

#endif // SCENEX_ASSET_H
