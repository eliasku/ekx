#include "display2d.h"
#include "sprite.h"
#include <ek/canvas.h>

ECX_DEFINE_TYPE(bounds2d_t);
ECX_DEFINE_TYPE(display2d_t);
ECX_DEFINE_TYPE(quad2d_t);
ECX_DEFINE_TYPE(sprite2d_t);
ECX_DEFINE_TYPE(ninepatch2d_t);
ECX_DEFINE_TYPE(arc2d_t);

static
void bounds2d_ctor(component_handle_t handle) {
    ((bounds2d_t*) Bounds2D.data[0])[handle] = (bounds2d_t) {
            rect_01(),
            0,
    };
}

static
void display2d_ctor(component_handle_t handle) {
    ((display2d_t*) Display2D.data[0])[handle] = (display2d_t) {0};
}

static
void sprite2d_ctor(component_handle_t handle) {
    sprite2d_t o = (sprite2d_t) {0};
    o.hit_pixels = true;
    ((sprite2d_t*) Sprite2D.data[0])[handle] = o;
}

static
void ninepatch2d_ctor(component_handle_t i) {
    ninepatch2d_t o = (ninepatch2d_t) {0};
    o.hit_pixels = true;
    ((ninepatch2d_t*) NinePatch2D.data[0])[i] = o;
}

static
void arc2d_ctor(component_handle_t i) {
    arc2d_t o = (arc2d_t) {0};
    o.radius = 10.0f;
    o.line_width = 10.0f;
    o.segments = 50;
    o.color_inner = COLOR_WHITE;
    o.color_outer = COLOR_WHITE;
    ((arc2d_t*) Arc2D.data[0])[i] = o;
}

void setup_display2d(void) {
    ECX_TYPE(bounds2d_t, 4);
    Bounds2D.ctor = bounds2d_ctor;

    ECX_TYPE(display2d_t, 256);
    Display2D.ctor = display2d_ctor;

    ECX_TYPE(quad2d_t, 32);
    ECX_TYPE(sprite2d_t, 32);
    Sprite2D.ctor = sprite2d_ctor;

    ECX_TYPE(ninepatch2d_t, 32);
    NinePatch2D.ctor = ninepatch2d_ctor;

    ECX_TYPE(arc2d_t, 4);
    Arc2D.ctor = arc2d_ctor;
}

rect_t get_world_rect(const bounds2d_t* bounds, mat3x2_t world_matrix) {
    const aabb2_t bb = aabb2_add_transformed_rect(aabb2_empty(), bounds->rect, world_matrix);
    return aabb2_get_rect(bb);
}

rect_t get_screen_rect(const bounds2d_t* bounds, mat3x2_t view_matrix, mat3x2_t world_matrix) {
    const mat3x2_t g = mat3x2_mul(view_matrix, world_matrix);
    const aabb2_t bb = aabb2_add_transformed_rect(aabb2_empty(), bounds->rect, g);
    return aabb2_get_rect(bb);
}

quad2d_t* quad2d_setup(entity_t e) {
    quad2d_t* drawable = add_quad2d(e);
    drawable->src = R_SPRITE_EMPTY;
    drawable->rect = rect_01();
    drawable->colors[0] = COLOR_WHITE;
    drawable->colors[1] = COLOR_WHITE;
    drawable->colors[2] = COLOR_WHITE;
    drawable->colors[3] = COLOR_WHITE;

    display2d_t* display = add_display2d(e);
    display->draw = quad2d_draw;
    display->hit_test = quad2d_hit_test;
    display->get_bounds = quad2d_get_bounds;
    return drawable;
}

void quad2d_draw(entity_t e) {
    quad2d_t* d = get_quad2d(e);
    const sprite_t* spr = &REF_RESOLVE(res_sprite, d->src);
    if (spr->state & SPRITE_LOADED) {
        const sg_image image = REF_RESOLVE(res_image, spr->image_id);
        if (image.id) {
            canvas_set_image_region(image, spr->tex);
        }
    } else {
        canvas_set_empty_image();
    }
    rect_t rc = d->rect;
    color_t* colors = d->colors;
    canvas_quad_color4(rc.x, rc.y, rc.w, rc.h, colors[0], colors[1], colors[2], colors[3]);
}

rect_t quad2d_get_bounds(entity_t e) {
    return get_quad2d(e)->rect;
}

bool quad2d_hit_test(entity_t e, vec2_t lp) {
    return rect_contains(get_quad2d(e)->rect, lp);
}

sprite2d_t* sprite2d_setup(entity_t e) {
    display2d_t* display = add_display2d(e);
    sprite2d_t* drawable = add_sprite2d(e);
    display->draw = sprite2d_draw;
    display->hit_test = sprite2d_hit_test;
    display->get_bounds = sprite2d_get_bounds;
    return drawable;
}

void sprite2d_draw(entity_t e) {
    R(sprite_t) src = get_sprite2d(e)->src;
    if (!src) {
        return;
    }
    const sprite_t* sprite = &REF_RESOLVE(res_sprite, src);
    draw_sprite(sprite);
}

rect_t sprite2d_get_bounds(entity_t e) {
    R(sprite_t) src = get_sprite2d(e)->src;
    if (src) {
        const sprite_t* sprite = &REF_RESOLVE(res_sprite, src);
        return sprite->rect;
    }
    return (rect_t) {0};
}

bool sprite2d_hit_test(entity_t e, vec2_t point) {
    if (rect_contains(sprite2d_get_bounds(e), point)) {
        sprite2d_t* d = get_sprite2d(e);
        if (d->hit_pixels && d->src) {
            const sprite_t* sprite = &REF_RESOLVE(res_sprite, d->src);
            return hit_test_sprite(sprite, point);
        }
        return true;
    }
    return false;
}

ninepatch2d_t* ninepatch2d_setup(entity_t e) {
    display2d_t* display = add_display2d(e);
    ninepatch2d_t* drawable = add_ninepatch2d(e);
    display->draw = ninepatch2d_draw;
    display->hit_test = ninepatch2d_hit_test;
    display->get_bounds = ninepatch2d_get_bounds;
    return drawable;
}

void ninepatch2d_draw(entity_t e) {
    const ninepatch2d_t* d = get_ninepatch2d(e);
    if (!d->src) {
        return;
    }
    sprite_t* spr = &REF_RESOLVE(res_sprite, d->src);
    canvas_save_matrix();
    canvas_scale(vec2(1.0f / d->scale.x, 1.0f / d->scale.y));
    // TODO: rotated
    rect_t target = d->manual_target;
    if (rect_is_empty(target)) {
        target = rect_scale(rect_expand(spr->rect, -1), d->scale);
    }
    draw_sprite_grid(spr, d->scale_grid, target);
    canvas_restore_matrix();
}

rect_t ninepatch2d_get_bounds(entity_t e) {
    R(sprite_t) src = get_ninepatch2d(e)->src;
    if (src) {
        return REF_RESOLVE(res_sprite, src).rect;
    }
    return (rect_t) {0};
}

bool ninepatch2d_hit_test(entity_t e, vec2_t point) {
    if (rect_contains(ninepatch2d_get_bounds(e), point)) {
        const ninepatch2d_t* d = get_ninepatch2d(e);
        if (d->hit_pixels && d->src) {
            return hit_test_sprite(&REF_RESOLVE(res_sprite, d->src), point);
        }
        return true;
    }
    return false;
}

arc2d_t* arc2d_setup(entity_t e) {
    display2d_t* display = add_display2d(e);
    arc2d_t* drawable = add_arc2d(e);
    display->draw = arc2d_draw;
    display->hit_test = arc2d_hit_test;
    display->get_bounds = arc2d_get_bounds;
    return drawable;
}

void arc2d_draw(entity_t e) {
    EK_ASSERT(is_entity(e));
    arc2d_t* d = (arc2d_t*) get_component(&Arc2D, e);
    if (!d->sprite) {
        return;
    }

    const sprite_t* spr = &REF_RESOLVE(res_sprite, d->sprite);
    const sg_image image = REF_RESOLVE(res_image, spr->image_id);
    if (!image.id) {
        return;
    }

    canvas_set_image(image);
    const rect_t tex = spr->tex;
    canvas_set_image_rect(rect(RECT_CENTER_X(tex), tex.y, 0.0f, tex.h));

    float off = -0.5f * MATH_PI;
    canvas_line_arc(0, 0, d->radius,
                    0 + off, d->angle + off,
                    d->line_width, d->segments,
                    d->color_inner, d->color_outer);
}

rect_t arc2d_get_bounds(entity_t e) {
    arc2d_t* d = (arc2d_t*) get_component(&Arc2D, e);
    float s = d->radius + d->line_width;
    return rect(-s, -s, 2.0f * s, 2.0f * s);
}

bool arc2d_hit_test(entity_t e, vec2_t point) {
    arc2d_t* d = (arc2d_t*) get_component(&Arc2D, e);
    const float len = length_vec2(point);
    const float r = d->radius;
    const float dr = d->line_width;
    return len >= r && len <= (r + dr);
}

/** utilities **/
void set_gradient_quad(entity_t e, rect_t rc, color_t top, color_t bottom) {
    quad2d_t* q = add_quad2d(e);
    q->src = R_SPRITE_EMPTY;
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    display2d_t* display = add_display2d(e);
    display->draw = quad2d_draw;
    display->hit_test = quad2d_hit_test;
    display->get_bounds = quad2d_get_bounds;
}

void set_color_quad(entity_t e, rect_t rc, color_t color) {
    set_gradient_quad(e, rc, color, color);
}
