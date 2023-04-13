#ifndef SCENEX_LAYOUT_RECT_H
#define SCENEX_LAYOUT_RECT_H


#include <ecx/ecx.h>
#include "transform2d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    vec2_t x;
    vec2_t y;
    rect_t fill_extra;
    rect_t rect;
    rect_t safeRect;
    bool fill_x;
    bool fill_y;
    bool align_x;
    bool align_y;
    bool doSafeInsets;
}  layout_rect_t;

rect_t find_parent_layout_rect(entity_t e, bool safe);

void LayoutRect_hard(entity_t e, float x, float y);
void LayoutRect_hard_x(entity_t e, float x);
void LayoutRect_hard_y(entity_t e, float y);


layout_rect_t * LayoutRect_enableAlignX(layout_rect_t * l,float relative, float absolute);
layout_rect_t * LayoutRect_enableAlignY(layout_rect_t * l,float relative, float absolute);
layout_rect_t * LayoutRect_aligned(layout_rect_t * l,float relativeX, float absoluteX, float relativeY, float absoluteY);
layout_rect_t * LayoutRect_fill(layout_rect_t * l,bool xAxis, bool yAxis);
layout_rect_t * LayoutRect_setInsetsMode(layout_rect_t * l,bool safe);
void LayoutRect_hard(entity_t e, float x, float y);
void LayoutRect_hard_x(entity_t e, float x);
void LayoutRect_hard_y(entity_t e, float y);

extern rect_t LayoutRect_design_canvas_rect;
extern ecx_component_type LayoutRect;
void LayoutRect_setup(void);
void LayoutRect_update(void);
#define get_layout_rect(e) ((layout_rect_t*)get_component(&LayoutRect, e))
#define add_layout_rect(e) ((layout_rect_t*)add_component(&LayoutRect, e))

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// wrapper

class layout_wrapper {
public:
    layout_wrapper(entity_t e) :
            e_{e},
            l_{add_layout_rect(e)} {
    }

    layout_wrapper& aligned(float rel_x = 0.0f, float abs_x = 0.0f, float rel_y = 0.0f, float abs_y = 0.0f) {
        horizontal(rel_x, abs_x);
        vertical(rel_y, abs_y);
        return *this;
    }

    layout_wrapper& hard(float x, float y) {
        LayoutRect_hard(e_, x, y);
        return *this;
    }

    layout_wrapper& hard_y(float y = 0.0f) {
        LayoutRect_hard_y(e_, y);
        return *this;
    }

    layout_wrapper& horizontal(float multiplier = 0.0f, float offset = 0.0f) {
        LayoutRect_enableAlignX(l_, multiplier, offset);
        return *this;
    }

    layout_wrapper& vertical(float multiplier = 0.0f, float offset = 0.0f) {
        LayoutRect_enableAlignY(l_, multiplier, offset);
        return *this;
    }

    layout_wrapper& fill(bool horizontal = true, bool vertical = true) {
        LayoutRect_fill(l_, horizontal, vertical);
        return *this;
    }

    layout_wrapper& fill_extra(const rect_t rc) {
        l_->fill_extra = rc;
        return *this;
    }

private:
    entity_t e_;
    layout_rect_t* l_;
};


#endif


#endif // SCENEX_LAYOUT_RECT_H
