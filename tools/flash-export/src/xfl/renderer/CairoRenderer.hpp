#pragma once

#include "../types.hpp"

typedef struct _cairo cairo_t;
typedef struct _cairo_pattern cairo_pattern_t;
typedef struct _cairo_surface cairo_surface_t;

typedef struct bitmap_data bitmap_data_t;

namespace ek::xfl {

struct RenderCommand;
struct FillStyle;
struct StrokeStyle;

class CairoRenderer {
public:
    explicit CairoRenderer(cairo_t* ctx);

    void execute(const RenderCommand& cmd);

    void set_transform(const TransformModel& transform);

    void draw_bitmap(const bitmap_data_t* bitmap);

private:
    cairo_t* ctx_;

    TransformModel transform_;

    bool fill_flag_ = false;
    bool stroke_flag_ = false;
    bool open_flag_ = false;

    const FillStyle* fill_style_ = nullptr;
    const StrokeStyle* stroke_style_ = nullptr;

    void open();

    void fill();

    void stroke();

    void close();

    void paint();
};

}
