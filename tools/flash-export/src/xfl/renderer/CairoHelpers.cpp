#include "CairoHelpers.hpp"
#include <cairo.h>

#include "../types.hpp"
#include <ek/math.h>

namespace ek::xfl {

cairo_matrix_t create_matrix(const mat3x2_t& m) {
    cairo_matrix_t cm;
    cm.xx = m.a;
    cm.yx = m.b;
    cm.xy = m.c;
    cm.yy = m.d;
    cm.x0 = m.tx;
    cm.y0 = m.ty;
    return cm;
}

void cairo_transform(cairo_t* cr, const mat3x2_t& m) {
    cairo_matrix_t transform_matrix = create_matrix(m);
    cairo_transform(cr, &transform_matrix);
}

cairo_line_cap_t convert_line_cap(LineCaps cap) {
    switch (cap) {
        case LineCaps::none:
            return CAIRO_LINE_CAP_BUTT;
        case LineCaps::round:
            return CAIRO_LINE_CAP_ROUND;
        case LineCaps::square:
            return CAIRO_LINE_CAP_SQUARE;
    }
}

cairo_line_join_t convert_line_join(LineJoints join) {
    switch (join) {
        case LineJoints::miter:
            return CAIRO_LINE_JOIN_MITER;
        case LineJoints::round:
            return CAIRO_LINE_JOIN_ROUND;
        case LineJoints::bevel:
            return CAIRO_LINE_JOIN_BEVEL;
    }
}

void set_line_cap(cairo_t* ctx, LineCaps cap) {
    cairo_set_line_cap(ctx, convert_line_cap(cap));
}

void set_line_join(cairo_t* ctx, LineJoints join) {
    cairo_set_line_join(ctx, convert_line_join(join));
}

void set_solid_fill(cairo_t* context, const vec4_t color) {
    cairo_set_source_rgba(context, color.x, color.y, color.z, color.w);
}

void set_blend_mode(cairo_t* ctx, BlendMode blend_mode) {
    cairo_operator_t cop{cairo_operator_t::CAIRO_OPERATOR_OVER};
    switch (blend_mode) {
        case BlendMode::multiply:
            cop = cairo_operator_t::CAIRO_OPERATOR_MULTIPLY;
            break;
        case BlendMode::screen:

            cop = cairo_operator_t::CAIRO_OPERATOR_SCREEN;
            break;
        case BlendMode::overlay:
            cop = cairo_operator_t::CAIRO_OPERATOR_OVERLAY;
            break;
        case BlendMode::hardlight:
            cop = cairo_operator_t::CAIRO_OPERATOR_HARD_LIGHT;
            break;
        case BlendMode::normal:
            break;
        case BlendMode::layer:
            break;
        case BlendMode::last:
            break;
        case BlendMode::lighten:
            cop = cairo_operator_t::CAIRO_OPERATOR_LIGHTEN;
            break;
        case BlendMode::darken:
            cop = cairo_operator_t::CAIRO_OPERATOR_DARKEN;
            break;
        case BlendMode::difference:
            cop = cairo_operator_t::CAIRO_OPERATOR_DIFFERENCE;
            break;
        case BlendMode::add:
            cop = cairo_operator_t::CAIRO_OPERATOR_ADD;
            break;
        case BlendMode::subtract:
            cop = cairo_operator_t::CAIRO_OPERATOR_EXCLUSION;
            break;
        case BlendMode::invert:
            cop = cairo_operator_t::CAIRO_OPERATOR_XOR; // ?
            break;
        case BlendMode::alpha:
            cop = cairo_operator_t::CAIRO_OPERATOR_ATOP; // ?
            break;
        case BlendMode::erase:
            cop = cairo_operator_t::CAIRO_OPERATOR_CLEAR; // ?
            break;
    }
    cairo_set_operator(ctx, cop);
}


void cairo_quadratic_curve_to(cairo_t* context, float x1, float y1, float x2, float y2) {
    double x = 0.0;
    double y = 0.0;
    cairo_get_current_point(context, &x, &y);

    if (0.0 == x && 0.0 == y) {
        x = x1;
        y = y1;
    }

    cairo_curve_to(context, x + 2.0 / 3.0 * (x1 - x), y + 2.0 / 3.0 * (y1 - y), x2 + 2.0 / 3.0 * (x1 - x2),
                   y2 + 2.0 / 3.0 * (y1 - y2), x2, y2);
}

void add_color_stops(cairo_pattern_t* pattern,
                     const Array<GradientEntry>& entries,
                     const color2f_t color_transform) {
    for (const auto& entry: entries) {
        const auto& color = color2f_transform(color_transform, entry.color);
        cairo_pattern_add_color_stop_rgba(pattern, entry.ratio, color.x, color.y, color.z, color.w);
    }
}

// https://github.com/lightspark/lightspark/blob/master/src/backends/graphics.cpp
cairo_pattern_t* create_linear_pattern(const mat3x2_t& matrix) {
    const auto p0 = vec2_transform({{-819.2f, 0.0f}}, matrix);
    const auto p1 = vec2_transform({{819.2f, 0.0f}}, matrix);
    return cairo_pattern_create_linear(p0.x, p0.y, p1.x, p1.y);
}

cairo_pattern_t* create_radial_pattern(const mat3x2_t& matrix) {
    const auto p0 = vec2_transform({{0.0f, 0.0f}}, matrix);
    const auto p1 = vec2_transform({{819.2f, 0.0f}}, matrix);
    const auto radius = distance_vec2(p0, p1);
    return cairo_pattern_create_radial(p0.x, p0.y, 0.0, p0.x, p0.y, radius);
}

void set_stroke_style(cairo_t* ctx, const StrokeStyle& stroke) {
    cairo_set_line_width(ctx, stroke.weight);
    set_line_cap(ctx, stroke.caps);
    set_line_join(ctx, stroke.joints);
    cairo_set_miter_limit(ctx, stroke.miterLimit);
}

fill_pattern_data_t create_fill_pattern(const FillStyle& fill, const TransformModel& transform) {
    cairo_pattern_t* pattern = nullptr;
    cairo_surface_t* surface = nullptr;
    switch (fill.type) {
        case FillType::linear:
//            pattern = create_linear_pattern(transform.matrix * fill.matrix);
            pattern = cairo_pattern_create_linear(-819.2, 0, 819.2, 0);
            add_color_stops(pattern, fill.entries, transform.color);
            break;
        case FillType::radial:
            //pattern = create_radial_pattern(transform.matrix * fill.matrix);
            pattern = cairo_pattern_create_radial(0, 0, 0, 0, 0, 819);
            add_color_stops(pattern, fill.entries, transform.color);
            break;
        case FillType::bitmap:
            if (fill.bitmap) {
                auto w = fill.bitmap->width;
                auto h = fill.bitmap->height;
                surface = cairo_image_surface_create_for_data(
                        const_cast<uint8_t*>(fill.bitmap->data.data()),
                        CAIRO_FORMAT_ARGB32, w, h, w * 4);
                pattern = cairo_pattern_create_for_surface(surface);
            }
            break;
        default:
            break;
    }

    if (pattern) {
        auto matrix = create_matrix(mat3x2_mul(transform.matrix, fill.matrix));
        cairo_matrix_invert(&matrix);
        cairo_pattern_set_matrix(pattern, &matrix);

        switch (fill.spreadMethod) {
            case SpreadMethod::extend:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
                break;
            case SpreadMethod::reflect:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
                break;
            case SpreadMethod::repeat:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
                break;
        }

        cairo_pattern_set_filter(pattern, CAIRO_FILTER_BEST);
    }

    return {pattern, surface};
}

void blit_downsample(cairo_t* ctx, cairo_surface_t* source, int w, int h, double upscale) {
    auto* pattern = cairo_pattern_create_for_surface(source);
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_BEST);
    cairo_save(ctx);
    cairo_identity_matrix(ctx);
    const double downscale = 1.0 / upscale;
    cairo_scale(ctx, downscale, downscale);
    cairo_set_source(ctx, pattern);
//    cairo_set_source_surface(ctx, source, 0, 0);
    cairo_rectangle(ctx, 0, 0, w, h);
    cairo_fill(ctx);
    cairo_restore(ctx);
    cairo_pattern_destroy(pattern);
}

void clear(cairo_t* ctx) {
    cairo_save(ctx);
    cairo_set_operator(ctx, CAIRO_OPERATOR_CLEAR);
    cairo_paint(ctx);
    cairo_restore(ctx);
}

void cairo_round_rectangle(cairo_t* cr, const double* values) {
    const double l = values[0];
    const double t = values[1];
    const double r = values[2];
    const double b = values[3];

    const double maxRadius = fmin((b - t) / 2, (r - l) / 2);

    const double r0 = CLAMP(values[4], -maxRadius, maxRadius);
    const double r1 = CLAMP(values[5], -maxRadius, maxRadius);
    const double r2 = CLAMP(values[6], -maxRadius, maxRadius);
    const double r3 = CLAMP(values[7], -maxRadius, maxRadius);

    double degrees = MATH_PI / 180.0;

    cairo_new_sub_path(cr);
    if (r1 >= 0) {
        cairo_arc(cr, r - r1, t + r1, r1, -90 * degrees, 0 * degrees);
    } else {
        // - add angle value
        // - set center to corner
        cairo_arc_negative(cr, r, t, -r1, -180 * degrees, -270 * degrees);
    }
    if (r2 >= 0) {
        cairo_arc(cr, r - r2, b - r2, r2, 0 * degrees, 90 * degrees);
    } else {
        cairo_arc_negative(cr, r, b, -r2, -90 * degrees, -180 * degrees);
    }
    if (r2 >= 0) {
        cairo_arc(cr, l + r3, b - r3, r3, 90 * degrees, 180 * degrees);
    } else {
        cairo_arc_negative(cr, l, b, -r3, 0 * degrees, -90 * degrees);
    }
    if (r0 >= 0) {
        cairo_arc(cr, l + r0, t + r0, r0, 180 * degrees, 270 * degrees);
    } else {
        cairo_arc_negative(cr, l, t, -r0, 90 * degrees, 0 * degrees);
    }
    cairo_close_path(cr);
}

void cairo_oval(cairo_t* cr, const double* values) {
    const double degrees = MATH_PI / 180.0;

    const double a0 = values[4];
    double a1 = values[5];
    if (a1 <= a0) {
        a1 += 360;
    }

    double sweep = a1 - a0;
    bool close = values[6] > 0;
    if (a1 == 360 && a0 == 0) {
        sweep = 360;
        close = false;
    }
    const double l = values[0];
    const double t = values[1];
    const double r = values[2];
    const double b = values[3];
    const double cx = l + (r - l) / 2;
    const double cy = t + (b - t) / 2;
    const double diameter = fmin(r - l, b - t);
    const double radius = diameter / 2;
    const double inner = radius * values[7];
    const double sx = (r - l) / diameter;
    const double sy = (b - t) / diameter;

    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, sx, sy);
    cairo_new_sub_path(cr);
    // path.arcTo({fLeft: l, fTop: t, fRight: r, fBottom: b}, a0, sweep, true);
    cairo_arc(cr, 0, 0, radius, degrees * a0, degrees * (a0 + sweep));
    //path.addArc([l, t, r, b], a0, sweep);
    if (inner > 0) {
        if (!close) {
            cairo_new_sub_path(cr);
        }
        cairo_arc_negative(cr, 0, 0, inner, degrees * (a0 + sweep), degrees * a0);

        if (close) {
            //path.arcToOval([l, t, r, b], a0, 0, false);
            cairo_close_path(cr);
        }
    } else {
        if (close) {
            cairo_line_to(cr, 0, 0);
            cairo_close_path(cr);
        }
    }
    cairo_restore(cr);
}

fill_pattern_data_t set_fill_style(cairo_t* cr, const FillStyle& fill, const TransformModel& transform) {
    fill_pattern_data_t pattern{};
    if (fill.type == FillType::solid) {
        set_solid_fill(cr, color2f_transform(transform.color, fill.entries[0].color));
    } else {
        pattern = create_fill_pattern(fill, transform);
    }

    if (pattern.pattern) {
        cairo_set_source(cr, pattern.pattern);
    }
    return pattern;
}

void fill_pattern_data_t::destroy() {
    if (pattern) {
        cairo_pattern_destroy(pattern);
        pattern = nullptr;
    }
    if (surface) {
        cairo_surface_destroy(surface);
        surface = nullptr;
    }
}
}