#include "atlas.h"

#include <ek/string.h>
#include <ek/log.h>
#include <ek/local_res.h>
#include <ek/gfx.h>
#include <ek/buf.h>
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

static void atlas_poll_loading(atlas_t* atlas);

void update_res_atlas(void) {
    for (res_id id = 0; id < res_atlas.rr.num; ++id) {
        atlas_t* atlas = &res_atlas.data[id];
        atlas_poll_loading(atlas);
    }
}

static int get_scale_num(float scale) {
    if (scale <= 1.0f) {
        return 1;
    } else if (scale <= 2.0f) {
        return 2;
    } else if (scale <= 3.0f) {
        return 3;
    }
    return 4;
}

static void load_atlas_meta(atlas_t* atlas, ek_local_res* lr) {
    log_debug("Decoding Atlas META");
    log_debug("Atlas Base Path: %s", str_get(atlas->base_path));

    // header
    calo_reader_t reader = INIT_ZERO;
    reader.p = lr->buffer;
    read_calo(&reader);
    atlas_info_t atlas_info = read_stream_atlas_info(&reader);

    arr_for (p_loader, atlas->loaders) {
        ek_texture_loader_destroy(*p_loader);
    }
    arr_reset((void**) &atlas->loaders);

    arr_for(page, atlas_info.pages) {
        res_id image_asset = R_IMAGE(H(page->image_path));

        sg_image* image = &REF_RESOLVE(res_image, image_asset);
        if (image->id) {
            log_debug("Destroy old page image %s", page->image_path);
            sg_destroy_image(*image);
            image->id = SG_INVALID_ID;
        }

        arr_push(atlas->pages, image_asset);
        ek_texture_loader_ptr loader = ek_texture_loader_create();
        arr_push(atlas->loaders, loader);
        arr_for(spr, page->sprites) {
            res_id ref = R_SPRITE(spr->name);
            arr_push(atlas->sprites, ref);

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
        log_debug("Load atlas page %s/%s", str_get(atlas->base_path), page_image_path);

        const uint32_t index = (uint32_t) (page - atlas_info.pages);
        ek_texture_loader* loader = atlas->loaders[index];
        loader->formatMask = atlas->format_mask;
        ek_texture_loader_set_path(&loader->basePath, str_get(atlas->base_path));
        loader->imagesToLoad = 1;
        ek_texture_loader_set_path(&loader->urls[0], page_image_path);
        ek_texture_loader_load(loader);
    }
}

static void on_atlas_res_loaded(ek_local_res* lr) {
    atlas_t* atlas = (atlas_t*) lr->userdata;
    atlas->state_flags |= 1;
    if (ek_local_res_success(lr)) {
        load_atlas_meta(atlas, lr);
    }
    ek_local_res_close(lr);
}

void atlas_load(atlas_t* atlas, const char* path, float scale_factor) {
    char meta_file_path[1024];
    ek_snprintf(meta_file_path, sizeof meta_file_path, "%s%dx.atlas", path, get_scale_num(scale_factor));

    char dir_buf[1024];
    ek_path_dirname(dir_buf, sizeof dir_buf, path);
    str_init_c_str(&atlas->base_path, dir_buf);

    atlas->state_flags = 0;
    ek_local_res_load(meta_file_path, on_atlas_res_loaded, atlas);
}

static void atlas_poll_loading(atlas_t* atlas) {
    bool meta_loaded = atlas->state_flags & 1;
    bool images_loaded = atlas->state_flags & 2;
    if (meta_loaded && !images_loaded) {
        uint32_t images_loading = arr_size(atlas->loaders);
        for (uint32_t i = 0; i < arr_size(atlas->loaders); ++i) {
            ek_texture_loader* loader = atlas->loaders[i];
            if (loader) {
                ek_texture_loader_update(loader);
                if (!loader->loading) {
                    if (loader->status == 0) {
                        const res_id image_id = R_IMAGE(H(loader->urls[0].path));
                        REF_RESOLVE(res_image, image_id) = loader->image;
                        ek_texture_loader_destroy(loader);
                        atlas->loaders[i] = NULL;
                    }
                    --images_loading;
                }
            } else {
                --images_loading;
            }
        }
        if (images_loading == 0) {
            arr_for (p_loader, atlas->loaders) {
                if (*p_loader) {
                    ek_texture_loader_destroy(*p_loader);
                }
            }
            arr_reset((void**) &atlas->loaders);
            atlas->state_flags |= 2;
        }
    }
}

void atlas_clear(atlas_t* atlas) {
    // TODO: idea with ref counting and when we can unload - just delete all unreferenced resources

    arr_for (page, atlas->pages) {
        sg_image* image = &REF_RESOLVE(res_image, *page);
        if (image->id) {
            sg_destroy_image(*image);
            image->id = SG_INVALID_ID;
        }
    }

    arr_for (ref, atlas->sprites) {
        sprite_t* spr = &REF_RESOLVE(res_sprite, *ref);
        spr->image_id = 0;
        spr->state = 0;
    }

    arr_reset((void**) &atlas->pages);
    arr_reset((void**) &atlas->sprites);
    arr_reset((void**) &atlas->loaders);
    arr_reset(&atlas->base_path);

    atlas->state_flags = 0;
}

