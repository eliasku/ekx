#include <sce/assets/asset.h>

#include <ek/audio.h>
#include <ek/buf.h>
#include <ek/hash.h>
#include <ek/local_res.h>
#include <ek/log.h>
#include <ek/print.h>
#include <sce/atlas.h>
#include <sce/dynamic_atlas.h>
#include <sce/localization.h>
#include <sce/sg.h>
#include <sce/font.h>
#include <ek/string.h>
#include <ek/texture_loader.h>
#include <ek/time.h>

static float base_progress(asset_t* asset) {
    return (asset && asset->state == ASSET_STATE_READY) ? 1.0f : 0.0f;
}

static void base_load(asset_t* asset) {
    if (asset->state == ASSET_STATE_INITIAL) {
        asset->state = ASSET_STATE_LOADING;
        (asset->type->do_load)(asset);
    }
}

static void base_unload(asset_t* asset) {
    if (asset->state == ASSET_STATE_READY) {
        (asset->type->do_unload)(asset);
        asset->state = ASSET_STATE_INITIAL;
    }
}

asset_type pack_asset;
asset_type audio_asset;
asset_type atlas_asset;
asset_type dynamic_atlas_asset;
asset_type scene_asset;
asset_type bmfont_asset;
asset_type ttf_asset;
asset_type texture_asset;
asset_type strings_asset;
asset_type asset_types[32];
uint32_t asset_types_num;

typedef struct {
    asset_t base;
    R(auph_buffer)
    res;
    const char* path;
    uint32_t flags;
} audio_asset_t;

typedef struct {
    asset_t base;
    R(atlas_ptr)
    res;
    const char* name;
    uint8_t loaded_scale;
    uint32_t format_mask;
} atlas_asset_t;

typedef struct {
    asset_t base;
    R(dynamic_atlas_ptr)
    res;
    uint32_t flags;
} dynamic_atlas_asset_t;

typedef struct {
    asset_t base;
    R(SGFile)
    res;
    const char* path;
} scene_asset_t;

typedef struct {
    asset_t base;
    const char* path;
    R(Font)
    res;
    // keep data instance
    ek_local_res local_res;
} bmfont_asset_t;

typedef struct {
    asset_t base;
    R(sg_image)
    res;
    ek_texture_loader* loader;
    image_data_t data;
    // by default always premultiply alpha,
    // currently for cube maps will be disabled
    // TODO: export level option
    // bool premultiply_alpha;
} texture_asset_t;

typedef struct {
    asset_t* asset;
    lang_name_t lang;
    ek_local_res lr;
} lang_loader_t;

typedef struct {
    asset_t base;
    const char* name;
    lang_loader_t loaders[LANG_MAX_COUNT];
    uint32_t loaded;
    uint32_t total;
} strings_asset_t;


typedef struct {
    asset_t base;
    R(font_t)
    res;
    float base_font_size;
    const char* path;
    string_hash_t glyph_cache;
} ttf_asset_t;

typedef struct {
    asset_t base;
    const char* name;
    asset_ptr* assets;
    uint32_t assets_loaded;
    // data and strings table parsed and stored in reader until we unload
    calo_reader_t reader;
    ek_local_res file;
    bool list_loaded;
} pack_asset_t;

static asset_t* alloc_asset(struct asset_type_* type) {
    asset_t* asset = (asset_t*)calloc(type->type_size, 1);
    asset->type = type;
    asset->weight = 1;
    return asset;
}

#define VAR_NEW_ASSET(T) T##_t* asset = (T##_t*)alloc_asset(&(T));

asset_t* unpack_asset(calo_reader_t* r, string_hash_t type) {
    asset_t* result = NULL;
    if (type == H("audio")) {
        log_trace("create asset: audio");
        VAR_NEW_ASSET(audio_asset);
        asset->res = R_AUDIO(read_u32(r));
        asset->flags = read_u32(r);
        asset->path = read_stream_string(r);
        result = &asset->base;
    } else if (type == H("scene")) {
        log_trace("create asset: scene");
        VAR_NEW_ASSET(scene_asset);
        asset->res = R_SG(read_u32(r));
        asset->path = read_stream_string(r);
        result = &asset->base;
    } else if (type == H("bmfont")) {
        log_trace("create asset: bmfont");
        VAR_NEW_ASSET(bmfont_asset);
        asset->res = R_FONT(read_u32(r));
        asset->path = read_stream_string(r);
        result = &asset->base;
    } else if (type == H("ttf")) {
        log_trace("create asset: ttf");
        VAR_NEW_ASSET(ttf_asset);
        asset->res = R_FONT(read_u32(r));
        asset->path = read_stream_string(r);
        asset->glyph_cache = read_u32(r);
        asset->base_font_size = read_f32(r);
        result = &asset->base;
    } else if (type == H("atlas")) {
        log_trace("create asset: atlas");
        VAR_NEW_ASSET(atlas_asset);
        asset->name = read_stream_string(r);
        asset->res = R_ATLAS(read_u32(r));
        asset->format_mask = read_u32(r);
        result = &asset->base;
    } else if (type == H("dynamic_atlas")) {
        log_trace("create asset: dynamic_atlas");
        VAR_NEW_ASSET(dynamic_atlas_asset);
        asset->res = R_DYNAMIC_ATLAS(read_u32(r));
        asset->flags = read_u32(r);
        result = &asset->base;
    } else if (type == H("texture")) {
        log_trace("create asset: texture");
        VAR_NEW_ASSET(texture_asset);
        asset->res = R_IMAGE(read_u32(r));
        asset->data = read_stream_image_data(r);
        asset->base.weight = (float)arr_size(asset->data.images);
        result = &asset->base;
    } else if (type == H("strings")) {
        log_trace("create asset: strings");
        VAR_NEW_ASSET(strings_asset);
        asset->name = read_stream_string(r);
        uint32_t langs_num = read_u32(r);
        lang_name_t langs[LANG_MAX_COUNT] = INIT_ZERO;
        // TODO:
        memcpy(langs, r->p, (sizeof langs[0]) * langs_num);
        r->p += (sizeof langs[0]) * langs_num;

        asset->total = langs_num;
        for (uint32_t i = 0; i < langs_num; ++i) {
            asset->loaders[i].lang = langs[i];
            asset->loaders[i].asset = &asset->base;
        }
        asset->base.weight = (float)langs_num;
        result = &asset->base;
    } else if (type == H("pack")) {
        log_trace("create asset: pack");
        VAR_NEW_ASSET(pack_asset);
        asset->name = read_stream_string(r);
        result = &asset->base;
    } else {
        for (uint32_t i = 0; i < asset_types_num; ++i) {
            asset_type* cls = &asset_types[i];
            if (type == cls->type_name) {
                result = alloc_asset(cls);
                if (cls->read) {
                    cls->read(result, r);
                }
                return result;
            }
        }
        log_error("asset: unknown `type` name hash");
    }
    return result;
}

static void pack_on_list_loaded(ek_local_res* lr) {
    pack_asset_t* pack = (pack_asset_t*)lr->userdata;
    if (ek_local_res_success(lr)) {
        pack->file = *lr;
        calo_reader_t reader = INIT_ZERO;
        reader.p = lr->buffer;
        const uint8_t* end = reader.p + lr->length;
        read_calo(&reader);
        while (reader.p < end) {
            string_hash_t type = read_u32(&reader);
            if (type) {
                asset_t* asset = unpack_asset(&reader, type);
                if (asset) {
                    arr_push(pack->assets, asset);
                    assets_add(asset);
                }
            } else {
                break;
            }
        }
        pack->reader = reader;
        pack->file = *lr;
    }
    // ready for loading
    pack->list_loaded = true;
}

static void pack_do_load(asset_t* base) {
    pack_asset_t* pack = (pack_asset_t*)base;
    pack->list_loaded = false;
    pack->assets_loaded = 0;
    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, pack->name);
    ek_local_res_load(full_path, pack_on_list_loaded, pack);
}

static void pack_do_unload(asset_t* base) {
    pack_asset_t* pack = (pack_asset_t*)base;
    arr_for(p_asset, pack->assets) {
        asset_t* asset = *p_asset;
        asset->type->unload(asset);
    }
    arr_reset(pack->assets);
    pack->assets_loaded = 0;
    pack->list_loaded = false;

    ek_local_res_close(&pack->file);
    free_table(&pack->reader.strings);
}

static bool is_time_budget_allow_start_next_job(uint64_t since) {
    return ek_ticks_to_sec(ek_ticks(&since)) < 0.008;
}

static void pack_poll(asset_t* base) {
    pack_asset_t* pack = (pack_asset_t*)base;
    if (pack->base.state != ASSET_STATE_LOADING || !pack->list_loaded) {
        return;
    }

    uint64_t timer = ek_ticks(NULL);

    uint32_t loaded_assets_num = 0;
    arr_for(p_asset, pack->assets) {
        asset_t* asset = *p_asset;
        //        const asset_state_t initial_state = asset->state;
        if (is_time_budget_allow_start_next_job(timer)) {
            if (asset->state == ASSET_STATE_INITIAL) {
                //log_debug("Loading BEGIN: %s", asset->name_.c_str());
                (asset->type->load)(asset);
            }
            if (asset->state == ASSET_STATE_LOADING) {
                if (asset->type->poll) {
                    (asset->type->poll)(asset);
                }
            }
        }
        if (asset->state == ASSET_STATE_READY) {
            //if (initial_state != ASSET_STATE_READY) {
            // log_debug("Loading END: %s", asset->name_.c_str());
            //}
            ++loaded_assets_num;
        }
    }

    if (!is_time_budget_allow_start_next_job(timer)) {
        log_debug("Assets loading jobs spend %d ms", (int)(1000 * ek_ticks_to_sec(ek_ticks(&timer))));
    }

    pack->assets_loaded = loaded_assets_num;
    if (loaded_assets_num >= arr_size(pack->assets)) {
        base->state = ASSET_STATE_READY;
    }
}

float pack_progress(asset_t* base) {
    pack_asset_t* pack = (pack_asset_t*)base;
    switch (base->state) {
        case ASSET_STATE_READY:
            return 1.0f;
        case ASSET_STATE_INITIAL:
            return 0.0f;
        case ASSET_STATE_LOADING:
            // calculate sub-assets progress
            if (arr_size(pack->assets)) {
                float acc = 0.0f;
                float total = 0.0f;
                arr_for(p_asset, pack->assets) {
                    asset_t* asset = *p_asset;
                    const float w = asset->weight;
                    if (w > 0.0f) {
                        acc += w * asset->type->progress(asset);
                        total += w;
                    }
                }
                if (total > 0.0f) {
                    return acc / total;
                }
            }
    }
    return 0.0f;
}

// audio

static void audio_do_load(asset_t* base) {
    audio_asset_t* audio = (audio_asset_t*)base;
    auph_buffer* buffer = &REF_RESOLVE(res_audio, audio->res);
    // if assertion is triggering - implement cleaning up the slot before loading
    EK_ASSERT(buffer->id == 0);

    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, audio->path);
    *buffer = auph_load(full_path, (audio->flags & 1) ? AUPH_FLAG_STREAM : 0);
}

static void audio_poll(asset_t* base) {
    audio_asset_t* audio = (audio_asset_t*)base;
    auph_buffer buffer = REF_RESOLVE(res_audio, audio->res);
    bool failed = !auph_is_active(buffer.id);
    bool completed = auph_is_buffer_loaded(buffer) || (audio->flags & 2);
    if (failed || completed) {
        if (failed) {
            log_debug("failed to load sound %s", audio->path);
        } else {
            log_debug("loaded audio sound %s", audio->path);
        };
        audio->base.state = ASSET_STATE_READY;
    } else {
        log_debug("poll audio sound loading %s", audio->path);
    }
}

static void audio_do_unload(asset_t* base) {
    audio_asset_t* audio = (audio_asset_t*)base;
    if (audio->res) {
        auph_buffer* buffer = &REF_RESOLVE(res_audio, audio->res);
        if (buffer->id && auph_is_active(buffer->id)) {
            auph_unload(*buffer);
        }
        buffer->id = 0;
    }
}

// atlas

static void atlas_load_(asset_t* base) {
    atlas_asset_t* asset = (atlas_asset_t*)base;
    if (base->state != ASSET_STATE_READY || asset->loaded_scale != asset_manager.scale_uid) {
        asset->loaded_scale = asset_manager.scale_uid;

        atlas_t* atlas = &REF_RESOLVE(res_atlas, asset->res);
        atlas_clear(atlas);
        atlas->format_mask = asset->format_mask;

        // do not switch to loading state, because after first load system does not poll pack's Asset objects
        base->state = ASSET_STATE_LOADING;

        char full_path[1024];
        ek_path_join(full_path, sizeof full_path, asset_manager.base_path, asset->name);
        atlas_load(atlas, full_path, asset_manager.scale_factor);
    }
}

static void atlas_poll(asset_t* base) {
    atlas_asset_t* asset = (atlas_asset_t*)base;
    if (base->state == ASSET_STATE_LOADING) {
        atlas_t* atlas = &REF_RESOLVE(res_atlas, asset->res);
        // we poll atlas loading / reloading in separated process with Atlas::pollLoading for each Res<Atlas>
        bool atlas_meta_loaded = atlas->state_flags & 1;
        if (atlas_meta_loaded) {
            bool images_loaded = atlas->state_flags & 2;
            if (images_loaded) {
                base->state = ASSET_STATE_READY;
            }
        }
    }
}

static float atlas_progress(asset_t* base) {
    atlas_asset_t* asset = (atlas_asset_t*)base;
    if (base->state == ASSET_STATE_LOADING) {
        atlas_t* atlas = &REF_RESOLVE(res_atlas, asset->res);
        float progress = (float)(atlas->state_flags & 1);
        const uint32_t pages_num = arr_size(atlas->pages);
        if (pages_num) {
            uint32_t pages_loaded = 0;
            arr_for(p_page, atlas->pages) {
                if (REF_RESOLVE(res_image, *p_page).id) {
                    ++pages_loaded;
                }
            }
            progress += (float)pages_loaded / (float)pages_num;
        }
        return progress;
    }
    return base_progress(base);
}

static void atlas_do_unload(asset_t* base) {
    atlas_asset_t* asset = (atlas_asset_t*)base;
    atlas_t* atlas = &REF_RESOLVE(res_atlas, asset->res);
    atlas_clear(atlas);
}

// dynamic atlas
static void dynamic_atlas_do_load(asset_t* base) {
    dynamic_atlas_asset_t* asset = (dynamic_atlas_asset_t*)base;
    const int page_size = dynamic_atlas_estimate_better_size(asset_manager.scale_factor, 512, 2048);
    dynamic_atlas_t* ptr = &REF_RESOLVE(res_dynamic_atlas, asset->res);
    EK_ASSERT(ptr->pages_ == NULL);
    dynamic_atlas_create(ptr, page_size, page_size, (asset->flags & 1) != 0, (asset->flags & 2) != 0);
    base->state = ASSET_STATE_READY;
}

static void dynamic_atlas_do_unload(asset_t* base) {
    dynamic_atlas_asset_t* asset = (dynamic_atlas_asset_t*)base;
    dynamic_atlas_t* ptr = &REF_RESOLVE(res_dynamic_atlas, asset->res);
    dynamic_atlas_destroy(ptr);
}

// scene
static void scene_on_file_loaded(ek_local_res* lr) {
    scene_asset_t* asset = (scene_asset_t*)lr->userdata;
    if (ek_local_res_success(lr)) {
        sg_file_t* file = &REF_RESOLVE(res_sg, asset->res);
        sg_load(file, lr->buffer, (uint32_t)lr->length);
    }
    ek_local_res_close(lr);
    asset->base.state = ASSET_STATE_READY;
}

static void scene_do_load(asset_t* base) {
    scene_asset_t* asset = (scene_asset_t*)base;
    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, asset->path);
    ek_local_res_load(full_path, scene_on_file_loaded, asset);
}

static void scene_do_unload(asset_t* base) {
    scene_asset_t* asset = (scene_asset_t*)base;
    sg_file_t* file = &REF_RESOLVE(res_sg, asset->res);
    arr_reset(file->library);
    arr_reset(file->linkages);
    arr_reset(file->scenes);
}

// bmfont

static void bmfont_on_file_loaded(ek_local_res* lr) {
    bmfont_asset_t* asset = (bmfont_asset_t*)lr->userdata;
    if (ek_local_res_success(lr)) {
        font_t* fnt = &REF_RESOLVE(res_font, asset->res);
        if (fnt->loaded) {
            EK_ASSERT(false && "Font is not unloaded before");
            font_destroy(fnt);
        }
        // keep lr instance
        asset->local_res = *lr;
        font_init(fnt, FONT_TYPE_BITMAP);
        font_load_bmfont(fnt, lr->buffer, lr->length);
    } else {
        ek_local_res_close(lr);
    }
    asset->base.state = ASSET_STATE_READY;
}

static void bmfont_do_load(asset_t* base) {
    bmfont_asset_t* asset = (bmfont_asset_t*)base;
    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, asset->path);
    ek_local_res_load(full_path, bmfont_on_file_loaded, asset);
}

static void bmfont_do_unload(asset_t* base) {
    bmfont_asset_t* asset = (bmfont_asset_t*)base;
    font_t* fnt = &REF_RESOLVE(res_font, asset->res);
    if (fnt->loaded) {
        font_destroy(fnt);
    }
    ek_local_res_close(&asset->local_res);
    asset->local_res.closeFunc = NULL;
}

// ttf

static void ttf_on_file_loaded(ek_local_res* lr) {
    ttf_asset_t* this_ = (ttf_asset_t*)lr->userdata;

    font_t* fnt = &REF_RESOLVE(res_font, this_->res);
    if (fnt->loaded) {
        EK_ASSERT(false && "Font is not unloaded before");
        font_destroy(fnt);
    }

    // `lr` ownership moves to font impl
    if (ek_local_res_success(lr)) {
        font_init_ttf(fnt, asset_manager.scale_factor, this_->base_font_size,
                      this_->glyph_cache);
        ttf_loadFromMemory(fnt, lr);
    } else {
        ek_local_res_close(lr);
    }

    this_->base.state = ASSET_STATE_READY;
}

static void ttf_do_load(asset_t* base) {
    ttf_asset_t* asset = (ttf_asset_t*)base;

    char full_path[1024];
    ek_path_join(full_path, sizeof full_path, asset_manager.base_path, asset->path);
    ek_local_res_load(full_path, ttf_on_file_loaded, asset);
}

static void ttf_do_unload(asset_t* base) {
    ttf_asset_t* asset = (ttf_asset_t*)base;

    font_t* fnt = &REF_RESOLVE(res_font, asset->res);
    if (fnt->loaded) {
        font_destroy(fnt);
    }
}

// image

static void texture_do_load(asset_t* base) {
    texture_asset_t* asset = (texture_asset_t*)base;
    asset->loader = ek_texture_loader_create();
    ek_texture_loader_set_path(&asset->loader->basePath, asset_manager.base_path);
    const int num = (int)arr_size(asset->data.images);
    EK_ASSERT(num > 0 && num <= EK_TEXTURE_LOADER_IMAGES_MAX_COUNT);
    for (int i = 0; i < num; ++i) {
        ek_texture_loader_set_path(asset->loader->urls + i, asset->data.images[i]);
    }
    asset->loader->imagesToLoad = num;
    if (asset->data.type == IMAGE_DATA_CUBE_MAP) {
        asset->loader->isCubeMap = true;
        asset->loader->premultiplyAlpha = false;
        asset->loader->formatMask = asset->data.format_mask;
    }
    ek_texture_loader_load(asset->loader);
    base->state = ASSET_STATE_LOADING;
}

static void texture_poll(asset_t* base) {
    texture_asset_t* asset = (texture_asset_t*)base;
    if (asset->loader) {
        if (asset->loader->loading) {
            ek_texture_loader_update(asset->loader);
        }

        if (!asset->loader->loading) {
            base->error = asset->loader->status;
            if (base->error == 0) {
                REF_RESOLVE(res_image, asset->res) = asset->loader->image;
            }
            base->state = ASSET_STATE_READY;
            ek_texture_loader_destroy(asset->loader);
            asset->loader = NULL;
        }
    }
}

static float texture_progress(asset_t* base) {
    texture_asset_t* asset = (texture_asset_t*)base;
    if (base->state == ASSET_STATE_LOADING) {
        return asset->loader ? asset->loader->progress : 0.0f;
    }
    return base_progress(base);
}

static void texture_do_unload(asset_t* base) {
    texture_asset_t* asset = (texture_asset_t*)base;
    if (asset->res) {
        sg_image* image = &REF_RESOLVE(res_image, asset->res);
        if (image->id) {
            sg_destroy_image(*image);
            image->id = SG_INVALID_ID;
        }
    }
}

// strings

static void strings_on_lang_loaded(ek_local_res* lr) {
    lang_loader_t* loader = (lang_loader_t*)lr->userdata;
    strings_asset_t* asset = (strings_asset_t*)loader->asset;
    if (ek_local_res_success(lr)) {
        loader->lr = *lr;
        add_lang(loader->lang, lr->buffer, (uint32_t)lr->length);
    } else {
        log_error("Strings resource not found: %s", loader->lang.str);
        asset->base.error = 1;
    }
    ++asset->loaded;
    if (asset->loaded >= asset->total) {
        asset->base.state = ASSET_STATE_READY;
    }
}

static void strings_do_load(asset_t* base) {
    strings_asset_t* asset = (strings_asset_t*)base;
    asset->loaded = 0;
    char lang_path[1024];
    for (uint32_t i = 0; i < asset->total; ++i) {
        lang_loader_t* loader = &asset->loaders[i];
        ek_snprintf(lang_path, sizeof lang_path, "%s/%s/%s.mo", asset_manager.base_path, asset->name, loader->lang.str);
        ek_local_res_load(lang_path, strings_on_lang_loaded, loader);
    }
}

static void strings_do_unload(asset_t* base) {
    UNUSED(base);
    // run each lang loader and close LR:
    // ek_local_res_close(&data);
}

// custom asset types

asset_type new_asset_type(string_hash_t type_name, uint32_t type_size) {
    asset_type type = {
        .load = base_load,
        .unload = base_unload,
        .progress = base_progress,
        .type_name = type_name,
        .type_size = type_size,
    };
    return type;
}

void add_asset_type(asset_type type) {
    asset_types[asset_types_num++] = type;
}

void init_asset_types(void) {
    pack_asset = NEW_ASSET_TYPE(pack);
    pack_asset.poll = pack_poll;
    pack_asset.do_load = pack_do_load;
    pack_asset.do_unload = pack_do_unload;
    pack_asset.progress = pack_progress;

    audio_asset = NEW_ASSET_TYPE(audio);
    audio_asset.poll = audio_poll;
    audio_asset.do_load = audio_do_load;
    audio_asset.do_unload = audio_do_unload;

    atlas_asset = NEW_ASSET_TYPE(atlas);
    atlas_asset.load = atlas_load_;
    atlas_asset.progress = atlas_progress;
    atlas_asset.poll = atlas_poll;
    atlas_asset.do_unload = atlas_do_unload;

    dynamic_atlas_asset = NEW_ASSET_TYPE(dynamic_atlas);
    dynamic_atlas_asset.do_load = dynamic_atlas_do_load;
    dynamic_atlas_asset.do_unload = dynamic_atlas_do_unload;

    scene_asset = NEW_ASSET_TYPE(scene);
    scene_asset.do_load = scene_do_load;
    scene_asset.do_unload = scene_do_unload;

    bmfont_asset = NEW_ASSET_TYPE(bmfont);
    bmfont_asset.do_load = bmfont_do_load;
    bmfont_asset.do_unload = bmfont_do_unload;

    ttf_asset = NEW_ASSET_TYPE(ttf);
    ttf_asset.do_load = ttf_do_load;
    ttf_asset.do_unload = ttf_do_unload;

    texture_asset = NEW_ASSET_TYPE(texture);
    texture_asset.progress = texture_progress;
    texture_asset.poll = texture_poll;
    texture_asset.do_load = texture_do_load;
    texture_asset.do_unload = texture_do_unload;

    strings_asset = NEW_ASSET_TYPE(strings);
    strings_asset.do_load = strings_do_load;
    strings_asset.do_unload = strings_do_unload;
}

static pack_asset_t* root_pack_asset;

void assets_root_pack_load(const char* filepath) {
    EK_ASSERT(!root_pack_asset);
    log_debug("load root pack asset, content scale: %d%%.", (int)(100 * asset_manager.scale_factor));
    root_pack_asset = (pack_asset_t*)alloc_asset(&pack_asset);
    root_pack_asset->name = filepath;
    assets_add(&root_pack_asset->base);
    root_pack_asset->base.type->load(&root_pack_asset->base);
}

void assets_root_pack_poll(void) {
    EK_ASSERT(root_pack_asset);
    root_pack_asset->base.type->poll(&root_pack_asset->base);
}

float assets_root_pack_progress(void) {
    EK_ASSERT(root_pack_asset);
    return root_pack_asset->base.type->progress(&root_pack_asset->base);
}
