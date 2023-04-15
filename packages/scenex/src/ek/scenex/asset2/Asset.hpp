#ifndef SCENEX_ASSET_H
#define SCENEX_ASSET_H

#include <ek/ds/PodArray.hpp>
#include <ek/ds/String.hpp>

typedef enum {
    ASSET_STATE_INITIAL = 0,
    ASSET_STATE_LOADING = 1,
    ASSET_STATE_READY = 2,
} asset_state_t;

struct Asset {
    Asset() = default;

    virtual ~Asset() = default;

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
    float weight_ = 1.0f;
};

typedef Asset* asset_ptr;

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

#ifdef __cplusplus
}
#endif

struct PackAsset : public Asset {
    explicit PackAsset(ek::String name);
    void do_load() override;
    void do_unload() override;

    void poll() override;
    [[nodiscard]] float getProgress() const override;

    ek::String name_;
    ek::PodArray<Asset*> assets;
    unsigned assetsLoaded = 0;
    bool assetListLoaded = false;
};

#endif // SCENEX_ASSET_H
