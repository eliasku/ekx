#include "Asset.hpp"

#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/audio.h>
#include <ek/local_res.h>
#include <ek/gfx.h>

// texture loading
#include <ek/texture_loader.h>

#include <ek/scenex/scene_factory.h>
#include <ek/scenex/2d/atlas.h>
#include <ek/scenex/3d/scene3d.h>

#include <ek/scenex/text/font.h>

#include <utility>
#include <ek/scenex/2d/dynamic_atlas.h>

#include <ekx/app/localization.h>

void load_asset(asset_ptr asset) {
    asset->load();
}

void unload_asset(asset_ptr asset) {
    asset->unload();
}

void delete_asset(asset_ptr asset) {
    delete asset;
}

asset_state_t get_asset_state(asset_ptr asset) {
    return asset->state;
}

asset_ptr unpack_asset(calo_reader_t* r, string_hash_t type);

class AudioAsset : public asset_ {
public:
    AudioAsset(string_hash_t name, ek::String filepath, uint32_t flags_) :
            res{R_AUDIO(name)},
            path_{std::move(filepath)},
            flags{flags_} {
    }

    void do_load() override {
        auph_buffer* buffer = &REF_RESOLVE(res_audio, res);
        // if assertion is triggering - implement cleaning up the slot before loading
        EK_ASSERT(buffer->id == 0);

        char full_path[1024];
        ek_path_join(full_path, sizeof full_path, asset_manager.base_path, path_.c_str());
        *buffer = auph_load(full_path, (flags & 1) ? AUPH_FLAG_STREAM : 0);
    }

    void poll() override {
        auto buffer = REF_RESOLVE(res_audio, res);
        auto failed = !auph_is_active(buffer.id);
        auto completed = auph_is_buffer_loaded(buffer) || (flags & 2);
        if (failed || completed) {
            if (failed) {
                log_debug("failed to load sound %s", path_.c_str());
            } else {
                log_debug("loaded audio sound %s", path_.c_str());
            };
            state = ASSET_STATE_READY;
        } else {
            log_debug("poll audio sound loading %s", path_.c_str());
        }
    }

    void do_unload() override {
        if (res) {
            auph_buffer* audio = &REF_RESOLVE(res_audio, res);
            if (audio->id && auph_is_active(audio->id)) {
                auph_unload(*audio);
            }
            audio->id = 0;
        }
    }

    R(auph_buffer) res;
    ek::String path_;
    uint32_t flags;
};

struct AtlasAsset : public asset_ {

    AtlasAsset(const char* name_, uint32_t format_mask_) : res{R_ATLAS(H(name_))},
                                                           name{name_} {
        // we need to load atlas image and atlas meta
        weight_ = 2;
        format_mask = format_mask_;
    }

    void load() override {
        if (state != ASSET_STATE_READY || loaded_scale_ != asset_manager.scale_uid) {
            loaded_scale_ = asset_manager.scale_uid;

            atlas_t* atlas = &REF_RESOLVE(res_atlas, res);
            atlas_clear(atlas);
            atlas->format_mask = format_mask;

            // do not switch to loading state, because after first load system does not poll pack's Asset objects
            state = ASSET_STATE_LOADING;

            char full_path[1024];
            ek_path_join(full_path, sizeof full_path, asset_manager.base_path, name.c_str());
            atlas_load(atlas, full_path, asset_manager.scale_factor);
        }
    }

    void poll() override {
        if (state == ASSET_STATE_LOADING) {
            atlas_t* atlas = &REF_RESOLVE(res_atlas, res);
            // we poll atlas loading / reloading in separated process with Atlas::pollLoading for each Res<Atlas>
            bool atlas_meta_loaded = atlas->state_flags & 1;
            if (atlas_meta_loaded) {
                bool images_loaded = atlas->state_flags & 2;
                if (images_loaded) {
                    state = ASSET_STATE_READY;
                }
            }
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == ASSET_STATE_LOADING) {
            atlas_t* atlas = &REF_RESOLVE(res_atlas, res);
            float progress = (float)(atlas->state_flags & 1);
            const uint32_t pages_num = arr_size(atlas->pages);
            if (pages_num) {
                uint32_t pages_loaded = 0;
                arr_for (p_page, atlas->pages) {
                    if (REF_RESOLVE(res_image, *p_page).id) {
                        ++pages_loaded;
                    }
                }
                progress += (float) pages_loaded / (float) pages_num;
            }
            return progress;
        }
        return asset_::getProgress();
    }

    void do_unload() override {
        atlas_t* atlas = &REF_RESOLVE(res_atlas, res);
        atlas_clear(atlas);
    }

    R(atlas_ptr) res;
    ek::String name;
    uint8_t loaded_scale_ = 0;
    uint32_t format_mask = 1;
};

struct DynamicAtlasAsset : public asset_ {

    DynamicAtlasAsset(string_hash_t name, uint32_t flags) : res{R_DYNAMIC_ATLAS(name)}, flags_{flags} {
    }

    // do not reload dynamic atlas, because references to texture* should be invalidated,
    // but current strategy not allow that
    void do_load() override {
        const int page_size = dynamic_atlas_estimate_better_size(asset_manager.scale_factor, 512, 2048);
        dynamic_atlas_t* ptr = &REF_RESOLVE(res_dynamic_atlas, res);
        EK_ASSERT(ptr->pages_ == NULL);
        dynamic_atlas_create(ptr, page_size, page_size, (flags_ & 1) != 0, (flags_ & 2) != 0);
        state = ASSET_STATE_READY;
    }

    void do_unload() override {
        dynamic_atlas_t* ptr = &REF_RESOLVE(res_dynamic_atlas, res);
        dynamic_atlas_destroy(ptr);
    }

    R(dynamic_atlas_ptr) res;
    uint32_t flags_ = 0;
};

struct SceneAsset : public asset_ {

    explicit SceneAsset(string_hash_t name, ek::String path) :
            res{R_SG(name)},
            path_{std::move(path)} {
    }

    void do_load() override {
        char full_path[1024];
        ek_path_join(full_path, sizeof full_path, asset_manager.base_path, path_.c_str());
        ek_local_res_load(full_path,
                          [](ek_local_res* lr) {
                              SceneAsset* this_ = (SceneAsset*) lr->userdata;
                              if (ek_local_res_success(lr)) {
                                  sg_file_t* file = &REF_RESOLVE(res_sg, this_->res);
                                  sg_load(file, lr->buffer, (uint32_t) lr->length);
                              }
                              ek_local_res_close(lr);
                              this_->state = ASSET_STATE_READY;
                          }, this);
    }

    void do_unload() override {
        sg_file_t* file = &REF_RESOLVE(res_sg, res);
        arr_reset((void**) &file->library);
        arr_reset((void**) &file->linkages);
        arr_reset((void**) &file->scenes);
    }

    R(SGFile) res;
    ek::String path_;
};

struct BitmapFontAsset : public asset_ {

    BitmapFontAsset(string_hash_t name, ek::String path) :
            res{R_FONT(name)},
            path_{std::move(path)} {
    }

    void do_load() override {
        char full_path[1024];
        ek_path_join(full_path, sizeof full_path, asset_manager.base_path, path_.c_str());
        ek_local_res_load(
                full_path,
                [](ek_local_res* lr) {
                    BitmapFontAsset* this_ = (BitmapFontAsset*) lr->userdata;
                    if (ek_local_res_success(lr)) {
                        font_t* fnt = &REF_RESOLVE(res_font, this_->res);
                        if (fnt->loaded_) {
                            EK_ASSERT(false && "Font is not unloaded before");
                            font_destroy(fnt);
                        }
                        // keep lr instance
                        this_->local_res = *lr;
                        font_init(fnt, FONT_TYPE_BITMAP);
                        font_load_bmfont(fnt, lr->buffer, lr->length);
                    } else {
                        ek_local_res_close(lr);
                    }
                    this_->state = ASSET_STATE_READY;
                }, this);
    }

    void do_unload() override {
        font_t* fnt = &REF_RESOLVE(res_font, res);
        if (fnt->loaded_) {
            font_destroy(fnt);
        }
        ek_local_res_close(&local_res);
        local_res.closeFunc = nullptr;
    }

    // keep data instance
    ek_local_res local_res;
    R(Font) res;
    ek::String path_;

};

struct ImageAsset : public asset_ {

    ImageAsset(string_hash_t name, image_data_t data) :
            res{R_IMAGE(name)},
            data_{data} {
        weight_ = (float) arr_size(data_.images);
    }

    void do_load() override {
        loader = ek_texture_loader_create();
        ek_texture_loader_set_path(&loader->basePath, asset_manager.base_path);
        const int num = (int) arr_size(data_.images);
        EK_ASSERT(num > 0 && num <= EK_TEXTURE_LOADER_IMAGES_MAX_COUNT);
        for (int i = 0; i < num; ++i) {
            ek_texture_loader_set_path(loader->urls + i, data_.images[i]);
        }
        loader->imagesToLoad = num;
        if (data_.type == IMAGE_DATA_CUBE_MAP) {
            loader->isCubeMap = true;
            loader->premultiplyAlpha = false;
            loader->formatMask = data_.format_mask;
        }
        ek_texture_loader_load(loader);
        state = ASSET_STATE_LOADING;
    }

    void poll() override {
        if (loader) {
            if (loader->loading) {
                ek_texture_loader_update(loader);
            }

            if (!loader->loading) {
                error = loader->status;
                if (error == 0) {
                    REF_RESOLVE(res_image, res) = loader->image;
                }
                state = ASSET_STATE_READY;
                ek_texture_loader_destroy(loader);
                loader = nullptr;
            }
        }
    }

    [[nodiscard]]
    float getProgress() const override {
        if (state == ASSET_STATE_LOADING) {
            return loader ? loader->progress : 0.0f;
        }
        return asset_::getProgress();
    }

    void do_unload() override {
        if (res) {
            sg_image* image = &REF_RESOLVE(res_image, res);
            if (image->id) {
                sg_destroy_image(*image);
                *image = {SG_INVALID_ID};
            }
        }
    }

    R(sg_image) res;
    ek_texture_loader* loader = nullptr;
    image_data_t data_{};
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    bool premultiplyAlpha = true;
};

struct StringsAsset : public asset_ {

    StringsAsset(ek::String name, const lang_name_t* langs, uint32_t langs_num) :
            name_{std::move(name)} {
        total = langs_num;
        for (uint32_t i = 0; i < langs_num; ++i) {
            loaders_[i].lang = langs[i];
            loaders_[i].asset = this;
        }
        weight_ = (float) langs_num;
    }

    void do_load() override {
        loaded = 0;
        char lang_path[1024];
        for (uint32_t i = 0; i < total; ++i) {
            auto* loader = &loaders_[i];
            ek_snprintf(lang_path, sizeof lang_path, "%s/%s/%s.mo", asset_manager.base_path, name_.c_str(),
                        loader->lang.str);
            ek_local_res_load(
                    lang_path,
                    [](ek_local_res* lr) {
                        lang_loader* loader_ = (lang_loader*) lr->userdata;
                        StringsAsset* asset = loader_->asset;
                        if (ek_local_res_success(lr)) {
                            loader_->lr = *lr;
                            add_lang(loader_->lang, lr->buffer, (uint32_t) lr->length);
                        } else {
                            log_error("Strings resource not found: %s", loader_->lang.str);
                            asset->error = 1;
                        }
                        ++asset->loaded;
                        if (asset->loaded >= asset->total) {
                            asset->state = ASSET_STATE_READY;
                        }
                    },
                    loader
            );
        }
    }

    void do_unload() override {
        // run each lang loader and close LR:
        // ek_local_res_close(&data);
    }

    struct lang_loader {
        StringsAsset* asset;
        lang_name_t lang;
        ek_local_res lr;
    };

    ek::String name_;
    lang_loader loaders_[LANG_MAX_COUNT];
    uint32_t loaded = 0;
    uint32_t total = 0;
};

struct ModelAsset : public asset_ {

    explicit ModelAsset(ek::String name) :
            name_{std::move(name)} {
    }

    void do_load() override {
        char full_path[1024];
        ek_snprintf(full_path, sizeof full_path, "%s/%s.model", asset_manager.base_path, name_.c_str());
        ek_local_res_load(
                full_path,
                [](ek_local_res* lr) {
                    ModelAsset* this_ = (ModelAsset*) lr->userdata;
                    if (ek_local_res_success(lr)) {
                        calo_reader_t reader = {};
                        reader.p = lr->buffer;
                        read_calo(&reader);
                        model3d_t model = read_stream_model3d(&reader);
                        RES_NAME_RESOLVE(res_mesh3d, H(this_->name_.c_str())) = static_mesh(model);
                        ek_local_res_close(lr);
                    } else {
                        log_error("MODEL resource not found: %s", this_->name_.c_str());
                        this_->error = 1;
                    }
                    this_->state = ASSET_STATE_READY;
                },
                this
        );
    }

    void do_unload() override {
        static_mesh_t* pp = &RES_NAME_RESOLVE(res_mesh3d, H(name_.c_str()));
        static_mesh_destroy(pp);
    }

    ek::String name_;
};

bool isTimeBudgetAllowStartNextJob(uint64_t since) {
    return ek_ticks_to_sec(ek_ticks(&since)) < 0.008;
}

struct TrueTypeFontAsset : public asset_ {

    TrueTypeFontAsset(string_hash_t name, ek::String path, string_hash_t glyphCache, float baseFontSize) :
            res{R_FONT(name)},
            baseFontSize_{baseFontSize},
            path_{std::move(path)},
            glyphCache_{glyphCache} {
    }

    void do_load() override {
        char full_path[1024];
        ek_path_join(full_path, sizeof full_path, asset_manager.base_path, path_.c_str());
        ek_local_res_load(
                full_path,
                [](ek_local_res* lr) {
                    TrueTypeFontAsset* this_ = (TrueTypeFontAsset*) lr->userdata;

                    font_t* fnt = &REF_RESOLVE(res_font, this_->res);
                    if (fnt->loaded_) {
                        EK_ASSERT(false && "Font is not unloaded before");
                        font_destroy(fnt);
                    }

                    // `lr` ownership moves to font impl
                    if (ek_local_res_success(lr)) {
                        font_init_ttf(fnt, asset_manager.scale_factor, this_->baseFontSize_,
                                      this_->glyphCache_);
                        ttf_loadFromMemory(fnt, lr);
                    } else {
                        ek_local_res_close(lr);
                    }

                    this_->state = ASSET_STATE_READY;
                }, this);
    }

    void do_unload() override {
        font_t* fnt = &REF_RESOLVE(res_font, res);
        if (fnt->loaded_) {
            font_destroy(fnt);
        }
    }

    R(Font) res;
    float baseFontSize_;
    ek::String path_;
    string_hash_t glyphCache_;
};

asset_ptr unpack_asset(calo_reader_t* r, string_hash_t type) {
    if (type == H("audio")) {
        string_hash_t name = read_u32(r);
        uint32_t flags = read_u32(r);
        const char* path = read_stream_string(r);
        log_debug("sound load: %s", path);
        return new AudioAsset(name, path, flags);
    } else if (type == H("scene")) {
        string_hash_t name = read_u32(r);
        const char* path = read_stream_string(r);
        return new SceneAsset(name, path);
    } else if (type == H("bmfont")) {
        string_hash_t name = read_u32(r);
        const char* path = read_stream_string(r);
        return new BitmapFontAsset(name, path);
    } else if (type == H("ttf")) {
        string_hash_t name = read_u32(r);
        const char* path = read_stream_string(r);
        string_hash_t glyphCache = read_u32(r);
        float baseFontSize = read_f32(r);
        return new TrueTypeFontAsset(name, path, glyphCache, baseFontSize);
    } else if (type == H("atlas")) {
        const char* name = read_stream_string(r);
        uint32_t formatMask = read_u32(r);
        return new AtlasAsset(name, formatMask);
    } else if (type == H("dynamic_atlas")) {
        string_hash_t name = read_u32(r);
        uint32_t flags = read_u32(r);
        return new DynamicAtlasAsset(name, flags);
    } else if (type == H("model")) {
        const char* name = read_stream_string(r);
        return new ModelAsset(name);
    } else if (type == H("texture")) {
        string_hash_t name = read_u32(r);
        image_data_t texData = read_stream_image_data(r);
        return new ImageAsset(name, texData);
    } else if (type == H("strings")) {
        const char* name = read_stream_string(r);
        uint32_t langs_num = read_u32(r);
        lang_name_t langs[LANG_MAX_COUNT] = {};
        // TODO:
        memcpy(langs, r->p, (sizeof langs[0]) * langs_num);
        r->p += (sizeof langs[0]) * langs_num;

        return new StringsAsset(name, langs, langs_num);
    } else if (type == H("pack")) {
        const char* name = read_stream_string(r);
        return new PackAsset(name);
    } else {
        EK_ASSERT(false && "asset: unknown `type` name hash");
    }
    return nullptr;
}

PackAsset::PackAsset(ek::String name) :
        name_{std::move(name)} {
}

void PackAsset::do_load() {
    assetListLoaded = false;
    assetsLoaded = 0;
    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, name_.c_str());
    ek_local_res_load(
            full_path,
            [](ek_local_res* lr) {
                PackAsset* this_ = (PackAsset*) lr->userdata;
                if (ek_local_res_success(lr)) {
                    calo_reader_t reader = {};
                    reader.p = lr->buffer;
                    const uint8_t* end = reader.p + lr->length;
                    read_calo(&reader);
                    while (reader.p < end) {
                        string_hash_t type = read_u32(&reader);
                        if (type) {
                            asset_ptr asset = unpack_asset(&reader, type);
                            if (asset) {
                                this_->assets.push_back(asset);
                                assets_add(asset);
                            }
                        } else {
                            break;
                        }
                    }
                    ek_local_res_close(lr);
                }
                // ready for loading
                this_->assetListLoaded = true;
            },
            this
    );
}

void PackAsset::do_unload() {
    for (auto asset: assets) {
        asset->unload();
    }
    assets.clear();
    assetsLoaded = 0;
    assetListLoaded = false;
}

void PackAsset::poll() {
    if (state != ASSET_STATE_LOADING || !assetListLoaded) {
        return;
    }

    uint64_t timer = ek_ticks(nullptr);

    unsigned numAssetsLoaded = 0;
    for (auto asset: assets) {
        const auto initialState = asset->state;
        if (asset->state == ASSET_STATE_INITIAL) {
            if (isTimeBudgetAllowStartNextJob(timer)) {
//                    log_debug("Loading BEGIN: %s", asset->name_.c_str());
                asset->load();
            }
        }
        if (asset->state == ASSET_STATE_LOADING) {
            if (isTimeBudgetAllowStartNextJob(timer)) {
                asset->poll();
            }
        }
        if (asset->state == ASSET_STATE_READY) {
            if (initialState != ASSET_STATE_READY) {
//                    log_debug("Loading END: %s", asset->name_.c_str());
            }
            ++numAssetsLoaded;
        }
    }

    if (!isTimeBudgetAllowStartNextJob(timer)) {
        uint64_t since = timer;
        double elapsed = ek_ticks_to_sec(ek_ticks(&since));
        log_info("Assets loading jobs spend %d ms", (int) (elapsed * 1000));
    }

    assetsLoaded = numAssetsLoaded;
    if (numAssetsLoaded >= assets.size()) {
        state = ASSET_STATE_READY;
    }
}

float PackAsset::getProgress() const {
    switch (state) {
        case ASSET_STATE_READY:
            return 1.0f;
        case ASSET_STATE_INITIAL:
            return 0.0f;
        case ASSET_STATE_LOADING:
            // calculate sub-assets progress
            if (!assets.empty()) {
                float acc = 0.0f;
                float total = 0.0f;
                for (auto asset: assets) {
                    const float w = asset->weight_;
                    acc += w * asset->getProgress();
                    total += w;
                }
                if (total > 0.0f) {
                    return acc / total;
                }
            }
    }
    return 0.0f;
}

