#include "scene_factory.h"

#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/base/node_api.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/interactiv.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <ekx/app/localization.h>

struct res_sg res_sg;

void setup_res_sg(void) {
    struct res_sg* R = &res_sg;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

entity_t create_node2d(string_hash_t tag) {
    entity_t e = ecs::create<node_t, transform2d_t, world_transform2d_t>();
    ecs::get<node_t>(e).tag = tag;
    return e;
}

entity_t create_node2d_in(entity_t parent, string_hash_t tag, int index) {
    entity_t e = create_node2d(tag);
    if (index == -1) {
        append(parent, e);
    } else if (index == 0) {
        prepend(parent, e);
    } else {
        // not implemented yet
        append(parent, e);
    }
    return e;
}

void sg_load(sg_file_t* out, const void* data, uint32_t size) {
    if (size > 0) {
        calo_reader_t reader = {0};
        reader.p = (uint8_t*)data;
        read_calo(&reader);
        *out = read_stream_sg_file(&reader);
    } else {
        log_error("SCENE LOAD: empty buffer");
    }
}

const sg_node_data_t * sg_get(const sg_file_t* sg, string_hash_t library_name) {
    // TODO: optimize access!
    uint32_t count = arr_size(sg->library);
    for (uint32_t i = 0; i < count; ++i) {
        sg_node_data_t* item = sg->library + i;
        if (item->library_name == library_name) {
            return item;
        }
    }
    return nullptr;
}

void apply(entity_t e, const sg_node_data_t* data) {
    using namespace ek;
    if (data->movie_target_id >= 0) {
        ecs::add<MovieClipTargetIndex>(e).key = data->movie_target_id;
    }

    {
        auto& transform = ecs::get<transform2d_t>(e);
        transform.setMatrix(data->matrix);
        transform.color.scale = color_vec4(data->color.scale);
        transform.color.offset = color_vec4(data->color.offset);
    }

    set_touchable(e, data->flags & SG_NODE_TOUCHABLE);
    set_visible(e, data->flags & SG_NODE_VISIBLE);
    if (data->name) {
        // override
        set_tag(e, data->name);
    }

    if (data->dynamic_text) {
        const sg_dynamic_text_* dynamic_text = data->dynamic_text;
        TextFormat format{dynamic_text->font, dynamic_text->size};
        format.alignment = dynamic_text->alignment;
        format.leading = dynamic_text->line_spacing;
        format.wordWrap = dynamic_text->word_wrap;
        format.layersCount = (int) arr_size(dynamic_text->layers);
        if (format.layersCount > 4) {
            format.layersCount = 4;
        }
        for (int i = 0; i < format.layersCount; ++i) {
            const sg_text_layer_t layer = dynamic_text->layers[i];
            format.layers[i].color = layer.color;
            format.layers[i].offset = layer.offset;
            format.layers[i].blurRadius = layer.blur_radius;
            format.layers[i].blurIterations = layer.blur_iterations;
            format.layers[i].strength = layer.strength;
        }

        Text2D* dtext = text2d_setup(e);

        dtext->c_str = dynamic_text->text;
        dtext->flags = ((dtext->flags >> 2) << 2);

        dtext->format = format;
        dtext->localized = is_localized(dynamic_text->text);
        dtext->adjustsFontSizeToFitBounds = dtext->localized;
        dtext->rect = dynamic_text->rect;
    }

    if (data->movie) {
        auto& mov = ecs::add<MovieClip>(e);
        mov.data = data->movie;
        mov.fps = data->movie->fps;
    }

    Sprite2D* sprite = ecs::try_get<Sprite2D>(e);
    NinePatch2D* ninePatch = ecs::try_get<NinePatch2D>(e);

    if (data->sprite && !sprite) {
        if (rect_is_empty(data->scale_grid)) {
            sprite = sprite2d_setup(e);
            new Sprite2D();
            sprite->src = R_SPRITE(data->sprite);
        } else {
            ninePatch = ninepatch2d_setup(e);
            ninePatch->src = R_SPRITE(data->sprite);
            ninePatch->scale_grid = data->scale_grid;
        }
    }

    if (ninePatch) {
        ninePatch->scale = mat3x2_get_scale(data->matrix);
    }

    if (data->flags & (SG_NODE_SCISSORS_ENABLED | SG_NODE_HIT_AREA_ENABLED | SG_NODE_BOUNDS_ENABLED)) {
        auto& bounds = ecs::add<Bounds2D>(e);
        bounds.rect = data->bounding_rect;
        if (data->flags & SG_NODE_SCISSORS_ENABLED) {
            bounds.flags |= BOUNDS_2D_SCISSORS;
        }
        if (data->flags &  SG_NODE_HIT_AREA_ENABLED) {
            bounds.flags |= BOUNDS_2D_HIT_AREA;
        }
        if (data->flags & SG_NODE_BOUNDS_ENABLED) {
            bounds.flags |= BOUNDS_2D_CULL;
        }
    }

    if (data->flags & SG_NODE_BUTTON) {
        interactive_add(e)->cursor = EK_MOUSE_CURSOR_BUTTON;
        ecs::add<Button>(e);
    }
}

entity_t create_and_merge(const sg_file_t* sg,
                          const sg_node_data_t * data,
                          const sg_node_data_t* over = nullptr) {
    using namespace ek;
    auto e = ecs::create<node_t, transform2d_t, world_transform2d_t>();
    if (data) {
        apply(e, data);
    }
    if (over) {
        apply(e, over);
    }
    if (data) {
        uint32_t children_count = arr_size(data->children);
        for (uint32_t i = 0; i < children_count; ++i) {
            const sg_node_data_t* child = data->children + i;
            entity_t child_entity = create_and_merge(sg, sg_get(sg, child->library_name), child);
            append_strict(e, child_entity);
        }
    }

    return e;
}

void extend_bounds(const sg_file_t* file, const sg_node_data_t* data, aabb2_t* boundsBuilder,
                   const mat3x2_t matrix) {
    sprite_t* spr = &REF_RESOLVE(res_sprite, data->sprite);
    if (spr->state & SPRITE_LOADED) {
        *boundsBuilder = aabb2_add_transformed_rect(*boundsBuilder, spr->rect, matrix);
    }
    uint32_t children_count = arr_size(data->children);
    for (uint32_t i = 0; i < children_count; ++i) {
        const sg_node_data_t* child = data->children + i;
        const sg_node_data_t* symbol = child->library_name != 0 ? sg_get(file, child->library_name) : child;
        extend_bounds(file, symbol, boundsBuilder, mat3x2_mul(matrix, child->matrix));
    }
}

entity_t sg_create(string_hash_t library, string_hash_t name, entity_t parent) {
    using namespace ek;
    entity_t result = NULL_ENTITY;
    R(SGFile) file_ref = R_SG(library);
    if (file_ref) {
        const sg_file_t* file = &REF_RESOLVE(res_sg, file_ref);
        const sg_node_data_t* data = sg_get(file, name);
        if (data) {
            result = create_and_merge(file, data);
            if (result.id && parent.id) {
                append_strict(parent, result);
            }
        } else {
            log_warn("SG Object (%s) not found in library %s", hsp_get(name), hsp_get(library));
        }
    } else {
        log_warn("SG not found: (%s)", hsp_get(library));
    }
    return result;
}

rect_t sg_get_bounds(string_hash_t library, string_hash_t name) {
    R(SGFile) file_ref = R_SG(library);
    if (file_ref) {
        sg_file_t* file = &REF_RESOLVE(res_sg, file_ref);
        const sg_node_data_t* data = sg_get(file, name);
        if (data) {
            aabb2_t bb = aabb2_empty();
            extend_bounds(file, data, &bb, data->matrix);
            return aabb2_get_rect(bb);
        }
    }
    return {};
}
