#include "RenderElement.hpp"

#include "../image_set.h"
#include "../xfl/renderer/Scanner.hpp"
#include "../xfl/renderer/CairoRenderer.hpp"
#include "../xfl/renderer/CairoHelpers.hpp"
#include "../xfl/renderer/RenderCommand.hpp"
#include "../xfl/Doc.hpp"
#include <cairo.h>

namespace ek::xfl {

sprite_data_t renderMultiSample(rect_t bounds,
                                const Array<RenderCommandsBatch>& batches,
                                const RenderElementOptions& options) {
    // x4 super-sampling
    const double upscale = 4.0;

    const double scale = options.scale;
    const bool fixed = options.width > 0 && options.height > 0;

    auto rc = bounds;
    if (!options.trim) {
        rc = rect_expand(rc, 1);
    }

    bitmap_t bitmap{0, 0, nullptr};
    const auto w = static_cast<int>(fixed ? options.width : ceil(rc.w * scale));
    const auto h = static_cast<int>(fixed ? options.height : ceil(rc.h * scale));
    const int stride = w * 4;

    if (w > 0 && h > 0) {
        bitmap_alloc(&bitmap, w, h);

        auto surf = cairo_image_surface_create_for_data((uint8_t*)bitmap.pixels,
                                                        CAIRO_FORMAT_ARGB32,
                                                        w, h, stride);
        auto cr = cairo_create(surf);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
        cairo_set_source_surface(cr, surf, 0, 0);

        const int upscale_w = (int)(w * upscale);
        const int upscale_h = (int)(h * upscale);

        auto sub_surf = cairo_surface_create_similar(surf,
                                                     CAIRO_CONTENT_COLOR_ALPHA,
                                                     upscale_w,
                                                     upscale_h);
        auto sub_cr = cairo_create(sub_surf);
        cairo_set_antialias(sub_cr, CAIRO_ANTIALIAS_NONE);

        {
            CairoRenderer sub_renderer{sub_cr};

            cairo_scale(sub_cr, scale * upscale, scale * upscale);

            if (!fixed) {
                cairo_translate(sub_cr, -rc.x, -rc.y);
            }

            for (const auto& batch : batches) {
                sub_renderer.set_transform(batch.transform);
                for (const auto& cmd : batch.commands) {
                    sub_renderer.execute(cmd);
                }
            }

            cairo_surface_flush(sub_surf);

            blit_downsample(cr, sub_surf, upscale_w, upscale_h, upscale);
            cairo_surface_flush(surf);
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        cairo_destroy(sub_cr);
        cairo_surface_destroy(sub_surf);

        // convert ARGB to ABGR
        bitmap_swizzle_xwzy(&bitmap);
    }

    sprite_data_t data = {};
    data.padding = 1;
    data.rc = rc;
    // TODO: recti_wh(w, h)
    data.source = {{0, 0, w, h}};
    data.bitmap = bitmap;

    return data;
}

sprite_data_t renderLowQuality(rect_t bounds,
                               const Array<RenderCommandsBatch>& batches,
                               const RenderElementOptions& options) {
    const double scale = options.scale;
    const bool fixed = options.width > 0 && options.height > 0;

    auto rc = bounds;
    if (!options.trim) {
        rc = rect_expand(rc, 1);
    }

    bitmap_t bitmap = {0, 0, nullptr};
    const auto w = static_cast<int>(fixed ? options.width : ceil(rc.w * scale));
    const auto h = static_cast<int>(fixed ? options.height : ceil(rc.h * scale));
    const int stride = w * 4;

    if (w > 0 && h > 0) {
        bitmap_alloc(&bitmap, w, h);

        auto surf = cairo_image_surface_create_for_data((uint8_t*)bitmap.pixels,
                                                        CAIRO_FORMAT_ARGB32,
                                                        w, h, stride);
        auto cr = cairo_create(surf);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
        cairo_set_source_surface(cr, surf, 0, 0);
        {
            CairoRenderer renderer{cr};

            cairo_scale(cr, scale, scale);

            if (!fixed) {
                cairo_translate(cr, -rc.x, -rc.y);
            }

            for (const auto& batch : batches) {
                renderer.set_transform(batch.transform);
                for (const auto& cmd : batch.commands) {
                    renderer.execute(cmd);
                }
            }
            cairo_surface_flush(surf);
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        // convert ARGB to ABGR
        bitmap_swizzle_xwzy(&bitmap);
    }

    sprite_data_t data = {};
    data.padding = 1;
    data.rc = rc;
    // TODO: recti_wh
    data.source = {{0, 0, w, h}};
    data.bitmap = bitmap;

    return data;
}

bool checkContainsOnlyBitmapOperations(const Array<RenderCommandsBatch>& batches) {
    for (const auto& batch : batches) {
        for (const auto& cmd : batch.commands) {
            if (cmd.op != RenderCommand::Operation::bitmap) {
                return false;
            }
        }
    }
    return true;
}

sprite_data_t renderElementBatches(rect_t bounds,
                                   const Array<RenderCommandsBatch>& batches,
                                   const RenderElementOptions& options) {
    if (checkContainsOnlyBitmapOperations(batches)) {
        return renderLowQuality(bounds, batches, options);
    }
    return renderMultiSample(bounds, batches, options);
}

sprite_data_t renderElement(const Doc& doc, const Element& el, const RenderElementOptions& options) {
    Scanner scanner{};
    scanner.draw(doc, el);

    auto spr = renderElementBatches(aabb2_get_rect(scanner.bounds), scanner.batches, options);

    // copy string
    arr_init_from((void**) &spr.name, 1, el.item.name.c_str(), el.item.name.size() + 1);

    return spr;
}

}