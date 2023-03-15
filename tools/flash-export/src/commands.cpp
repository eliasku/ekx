#include "image_set.h"
#include "xfl/Doc.hpp"
#include "sxg/SGBuilder.hpp"
#include "sxg/RenderElement.hpp"

extern "C" void export_xfl(const char* xfl_path, image_set_t* image_set, const char* output_path, const char* output_images_path) {
    ek::xfl::Doc ff{xfl_path};
    ek::xfl::SGBuilder fe{ff};
    fe.build_library();
    fe.build_sprites(image_set);

    sg_file_t sg_data = fe.export_library();
    calo_writer_t writer = new_writer(100);
    write_stream_sg_file(&writer, sg_data);

    FILE* f = fopen(output_path, "wb");
    fwrite_calo(f, &writer);
    fclose(f);

    save(image_set, output_images_path);
}

///// prerender flash symbol

// INPUT SYMBOL [Scale WIDTH HEIGHT ALPHA TRIM OUTPUT_PATH]
extern "C" void runFlashFilePrerender(int argc, char** argv) {
    ek::xfl::Doc ff{argv[0]};
    ek::xfl::SGBuilder exporter{ff};
    auto& doc = exporter.doc;

    auto* item = doc.findLinkage(argv[1]);
    if (item) {
        int i = 2;
        while (i < argc) {
            const float scale = strtof(argv[i], nullptr);
            const int width = atoi(argv[i + 1]);
            const int height = atoi(argv[i + 2]);
            const bool alpha = atoi(argv[i + 3]) != 0;
            const bool trim = atoi(argv[i + 4]) != 0;
            const char* output = argv[i + 5];

            ek::xfl::RenderElementOptions opts{scale, width, height, alpha, trim};
            auto spr = renderElement(doc, *item, opts);
            ek_bitmap_save_png(&spr.bitmap, output, alpha);
            if (spr.bitmap.pixels) {
                bitmap_free(&spr.bitmap);
            }
            i += 6;
        }
    }
}