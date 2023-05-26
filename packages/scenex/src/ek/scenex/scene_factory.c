#include "scene_factory.h"

#include <ek/log.h>
#include <ek/buf.h>

#include <sce/sg.h>
#include <sce/sprite.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/transform2d.h>
#include <ek/scenex/2d/movieclip.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/2d/display2d.h>
#include <ek/scenex/2d/text2d.h>
#include <sce/localization.h>

entity_t create_node2d(string_hash_t tag) {
    entity_t e = create_entity();
    add_node(e)->tag = tag;
    add_transform2d(e);
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

void apply(entity_t e, const sg_node_data_t* data) {
    if (data->movie_target_id >= 0) {
        set_movieclip_target(e, data->movie_target_id);
    }

    {
        transform2d_t* transform = get_transform2d(e);
        transform2d_set_matrix(transform, data->matrix);
        transform->color.scale = color_vec4(data->color.scale);
        transform->color.offset = color_vec4(data->color.offset);
    }

    set_touchable(e, data->flags & SG_NODE_TOUCHABLE);
    set_visible(e, data->flags & SG_NODE_VISIBLE);
    if (data->name) {
        // override
        set_tag(e, data->name);
    }

    if (data->dynamic_text) {
        const sg_dynamic_text_t* dynamic_text = data->dynamic_text;
        text_format_t format = text_format(dynamic_text->font, dynamic_text->size);
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
            format.layers[i].visible = true;
        }

        text2d_t* dtext = text2d_setup(e);

        dtext->c_str = dynamic_text->text;
        dtext->flags = ((dtext->flags >> 2) << 2);

        dtext->format = format;
        dtext->localized = is_localized(dynamic_text->text);
        dtext->adjustsFontSizeToFitBounds = dtext->localized;
        dtext->rect = dynamic_text->rect;
    }

    if (data->movie) {
        movieclip_t* mc = add_movieclip(e);
        mc->data = data->movie;
        mc->fps = data->movie->fps;
    }

    sprite2d_t* sprite = get_sprite2d(e);
    ninepatch2d_t* ninepatch = get_ninepatch2d(e);

    if (data->sprite && !sprite) {
        if (rect_is_empty(data->scale_grid)) {
            sprite = sprite2d_setup(e);
            sprite->src = R_SPRITE(data->sprite);
        } else {
            ninepatch = ninepatch2d_setup(e);
            ninepatch->src = R_SPRITE(data->sprite);
            ninepatch->scale_grid = data->scale_grid;
        }
    }

    if (ninepatch) {
        ninepatch->scale = mat3x2_get_scale(data->matrix);
    }

    if (data->flags & (SG_NODE_SCISSORS_ENABLED | SG_NODE_HIT_AREA_ENABLED | SG_NODE_BOUNDS_ENABLED)) {
        bounds2d_t* bounds = add_bounds2d(e);
        bounds->rect = data->bounding_rect;
        if (data->flags & SG_NODE_SCISSORS_ENABLED) {
            bounds->flags |= BOUNDS_2D_SCISSORS;
        }
        if (data->flags & SG_NODE_HIT_AREA_ENABLED) {
            bounds->flags |= BOUNDS_2D_HIT_AREA;
        }
        if (data->flags & SG_NODE_BOUNDS_ENABLED) {
            bounds->flags |= BOUNDS_2D_CULL;
        }
    }

    if (data->flags & SG_NODE_BUTTON) {
        add_interactive(e)->cursor = EK_MOUSE_CURSOR_BUTTON;
        add_button(e);
    }
}

entity_t create_and_merge(const sg_file_t* sg,
                          const sg_node_data_t* data,
                          const sg_node_data_t* over) {
    entity_t e = create_entity();
    add_node(e);
    add_transform2d(e);
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
    entity_t result = NULL_ENTITY;
    R(SGFile) file_ref = R_SG(library);
    if (file_ref) {
        const sg_file_t* file = &REF_RESOLVE(res_sg, file_ref);
        const sg_node_data_t* data = sg_get(file, name);
        if (data) {
            result = create_and_merge(file, data, NULL);
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
    return rect_wh(0, 0);
}
