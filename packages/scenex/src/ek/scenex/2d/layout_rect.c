#include "layout_rect.h"
#include "display2d.h"
#include <ek/scenex/base/node.h>

rect_t LayoutRect_design_canvas_rect;
ecx_component_type LayoutRect;

static void LayoutRect_ctor(component_handle_t i) {
    ((layout_rect_t*) LayoutRect.data[0])[i] = (layout_rect_t) {
            .doSafeInsets = true,
    };
}

void LayoutRect_setup(void) {
    LayoutRect_design_canvas_rect = rect_01();
    init_component_type(&LayoutRect, (ecx_component_type_decl) {
            "LayoutRect", 32, 1, {sizeof(layout_rect_t)}
    });
    LayoutRect.ctor = LayoutRect_ctor;
}

rect_t find_parent_layout_rect(entity_t e, bool safe) {
    entity_t it = get_parent(e);
    while (it.id) {
        const layout_rect_t* layout = get_layout_rect(it);
        if (layout && !rect_is_empty(layout->rect)) {
            return safe ? layout->safeRect : layout->rect;
        }
        it = get_parent(it);
    }
    return rect_01();
}

// system
static void update_layout(entity_t e) {
    const layout_rect_t* l = get_layout_rect(e);
    rect_t top_rect = find_parent_layout_rect(e, l->doSafeInsets);
    if (UNLIKELY(rect_is_empty(top_rect))) {
        return;
    }
    transform2d_t* transform = get_transform2d(e);
    if (transform) {
        display2d_t* display = get_display2d(e);
        if ((l->fill_x || l->fill_y) && display) {
            quad2d_t* quad = get_quad2d(e);
            if (quad) {
                if (l->fill_x) {
                    quad->rect.x = top_rect.x;
                    quad->rect.w = top_rect.w;
                }
                if (l->fill_y) {
                    quad->rect.y = top_rect.y;
                    quad->rect.h = top_rect.h;
                }
            } else if (get_sprite2d(e)) {
                rect_t bounds = sprite2d_get_bounds(e);
                if (!rect_is_empty(bounds) && (l->fill_x || l->fill_y)) {
                    vec2_t pos = transform->pos;
                    vec2_t scale = transform->cached_scale;
                    if (l->fill_x) {
                        pos.x = top_rect.x + l->fill_extra.x;
                        scale.x = (top_rect.w + l->fill_extra.w) / bounds.w;
                    }
                    if (l->fill_y) {
                        pos.y = top_rect.y + l->fill_extra.y;
                        scale.y = (top_rect.h + l->fill_extra.h) / bounds.h;
                    }
                    transform->pos = pos;
                    transform2d_set_scale(transform, scale);
                }
            }
        }

        if (l->align_x) {
            transform->x = top_rect.x + l->x.y + l->x.x * top_rect.w;
        }

        if (l->align_y) {
            transform->y = top_rect.y + l->y.y + l->y.x * top_rect.h;
        }
    }
}

void LayoutRect_update(void) {
    for (uint32_t i = 1; i < LayoutRect.size; ++i) {
        entity_t e = get_entity(&LayoutRect, i);
        update_layout(e);
    }
}

layout_rect_t* LayoutRect_enableAlignX(layout_rect_t* l, float relative, float absolute) {
    l->align_x = true;
    l->x.x = relative;
    l->x.y = absolute;
    return l;
}

layout_rect_t* LayoutRect_enableAlignY(layout_rect_t* l, float relative, float absolute) {
    l->align_y = true;
    l->y.x = relative;
    l->y.y = absolute;
    return l;
}

layout_rect_t*
LayoutRect_aligned(layout_rect_t* l, float relativeX, float absoluteX, float relativeY, float absoluteY) {
    LayoutRect_enableAlignX(l, relativeX, absoluteX);
    LayoutRect_enableAlignY(l, relativeY, absoluteY);
    return l;
}

layout_rect_t* LayoutRect_fill(layout_rect_t* l, bool xAxis, bool yAxis) {
    l->fill_x = xAxis;
    l->fill_y = yAxis;
    return l;
}

layout_rect_t* LayoutRect_setInsetsMode(layout_rect_t* l, bool safe) {
    l->doSafeInsets = safe;
    return l;
}


void LayoutRect_hard(entity_t e, float x, float y) {
    if (e.id) {
        const vec2_t pos = add_transform2d(e)->pos;

        LayoutRect_aligned(add_layout_rect(e),
                           x, pos.x - (LayoutRect_design_canvas_rect.x + LayoutRect_design_canvas_rect.w * x),
                           y, pos.y - (LayoutRect_design_canvas_rect.y + LayoutRect_design_canvas_rect.h * y)
        );
    }
}

void LayoutRect_hard_x(entity_t e, float x) {
    const vec2_t pos = add_transform2d(e)->pos;
    LayoutRect_enableAlignX(add_layout_rect(e),
                            x, pos.x - (LayoutRect_design_canvas_rect.x + LayoutRect_design_canvas_rect.w * x)
    );
}

void LayoutRect_hard_y(entity_t e, float y) {
    const vec2_t pos = add_transform2d(e)->pos;
    LayoutRect_enableAlignY(add_layout_rect(e),
                            y, pos.y - (LayoutRect_design_canvas_rect.y + LayoutRect_design_canvas_rect.h * y)
    );
}
