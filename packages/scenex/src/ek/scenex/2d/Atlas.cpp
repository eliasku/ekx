#include "Atlas.hpp"
#include "sprite.h"

#include <ek/log.h>
#include <ek/local_res.h>
#include <ek/gfx.h>
#include <ek/print.h>
#include <gen_sg.h>

struct res_atlas res_atlas;

void setup_res_atlas(void) {
    struct res_atlas* R = &res_atlas;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void update_res_atlas(void) {
    for (res_id id = 0; id < res_atlas.rr.num; ++id) {
        atlas_ptr content = res_atlas.data[id];
        if (content) {
            content->pollLoading();
        }
    }
}

int get_scale_num(float scale) {
    if (scale <= 1.0f) {
        return 1;
    } else if (scale <= 2.0f) {
        return 2;
    } else if (scale <= 3.0f) {
        return 3;
    }
    return 4;
}

Atlas::Atlas() = default;

Atlas::~Atlas() {
    clear();
}

void load_atlas_meta(Atlas* atlas, ek_local_res* lr) {
    log_debug("Decoding Atlas META");
    log_debug("Atlas Base Path: %s", atlas->base_path.c_str());

    // header
    calo_reader_t reader = {0};
    reader.p = lr->buffer;
    read_calo(&reader);
    atlas_info_t atlas_info = read_stream_atlas_info(&reader);

    for (auto* loader: atlas->loaders) {
        ek_texture_loader_destroy(loader);
    }
    atlas->loaders.clear();

    arr_for(page, atlas_info.pages) {
        res_id image_asset = R_IMAGE(H(page->image_path));

        sg_image* image = &REF_RESOLVE(res_image, image_asset);
        if (image->id) {
            log_debug("Destroy old page image %s", page->image_path);
            sg_destroy_image(*image);
            *image = {SG_INVALID_ID};
        }

        atlas->pages.push_back(image_asset);
        atlas->loaders.emplace_back(ek_texture_loader_create());
        arr_for(spr, page->sprites) {
            res_id ref = R_SPRITE(spr->name);
            atlas->sprites.push_back(ref);

            sprite_t* sprite = &REF_RESOLVE(res_sprite, ref);
            EK_ASSERT((sprite->state & SPRITE_LOADED) == 0);
            sprite->state = spr->flags | SPRITE_LOADED;
            sprite->image_id = image_asset;
            sprite->rect = spr->rc;
            sprite->tex = spr->uv;
        }
    }

    arr_for(page, atlas_info.pages) {
        const char* page_image_path = page->image_path;
        log_debug("Load atlas page %s/%s", atlas->base_path.c_str(), page_image_path);

        const uint32_t index = (uint32_t)(page - atlas_info.pages);
        ek_texture_loader* loader = atlas->loaders[index];
        loader->formatMask = atlas->formatMask;
        ek_texture_loader_set_path(&loader->basePath, atlas->base_path.c_str());
        loader->imagesToLoad = 1;
        ek_texture_loader_set_path(&loader->urls[0], page_image_path);
        ek_texture_loader_load(loader);
    }
}

void Atlas::load(const char* path, float scaleFactor) {
    char meta_file_path[1024];
    ek_snprintf(meta_file_path, sizeof meta_file_path, "%s%dx.atlas", path, get_scale_num(scaleFactor));

    char dir_buf[1024];
    ek_path_dirname(dir_buf, sizeof dir_buf, path);
    base_path = dir_buf;

    ek_local_res_load(
            meta_file_path,
            [](ek_local_res* lr) {
                Atlas* this_ = (Atlas*) lr->userdata;
                if (ek_local_res_success(lr)) {
                    load_atlas_meta(this_, lr);
                }
                ek_local_res_close(lr);
            },
            this
    );
}

int Atlas::pollLoading() {
    int toLoad = (int) loaders.size();
    if (toLoad > 0) {
        for (uint32_t i = 0; i < loaders.size(); ++i) {
            auto* loader = loaders[i];
            if (loader) {
                ek_texture_loader_update(loader);
                if (!loader->loading) {
                    if (loader->status == 0) {
                        // ref = ek_ref_make(sg_image, loader->urls[0].path)
                        // ek_ref_clear(ref)
                        // item = ek_ref_get_item(ref)
                        // item->handle = loader->image;
                        // item->finalizer = sg_image_REF_finalizer
                        const res_id image_id = R_IMAGE(H(loader->urls[0].path));
                        REF_RESOLVE(res_image, image_id) = loader->image;
                        ek_texture_loader_destroy(loader);
                        loaders[i] = nullptr;
                    }
                    --toLoad;
                }
            } else {
                --toLoad;
            }
        }
        if (toLoad == 0) {
            for (auto* loader: loaders) {
                if (loader) {
                    ek_texture_loader_destroy(loader);
                }
            }
            loaders.clear();
            return 0;
        } else {
            return toLoad;
        }
    }
    return 0;
}

int Atlas::getLoadingImagesCount() const {
    int loading = 0;
    for (uint32_t i = 0; i < loaders.size(); ++i) {
        auto* loader = loaders[i];
        if (loader) {
            ++loading;
        }
    }
    return loading;
}

void Atlas::clear() {
    // TODO: idea with ref counting and when we can unload - just delete all unreferenced resources

    for (auto page: pages) {
        sg_image* image = &REF_RESOLVE(res_image, page);
        if (image->id) {
            sg_destroy_image(*image);
            *image = {SG_INVALID_ID};
        }
    }

    for (auto ref: sprites) {
        auto* spr = &REF_RESOLVE(res_sprite, ref);
        spr->image_id = 0;
        spr->state = 0;
    }
}

