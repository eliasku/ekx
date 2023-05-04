#include "../types.hpp"

#include <calo.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <stb/stb_image.h>
#include <miniz.h>

// thanks to https://github.com/charrea6/flash-hd-upscaler/blob/master/images.py

const uint16_t SIGNATURE_JPEG = 0xD8FF;
const uint16_t SIGNATURE_ARGB = 0x0503;
const uint16_t SIGNATURE_CLUT = 0x0303;

typedef struct {
    uint16_t stride;
    uint16_t width;
    uint16_t height;
    uint32_t width_high;
    uint32_t width_tw;
    uint32_t height_high;
    uint32_t height_tw;
    uint8_t flags;
} BitmapItemHeader;

BitmapItemHeader read_bitmap_item_header(calo_reader_t* r) {
    BitmapItemHeader desc{};
    desc.stride = read_u16(r);
    desc.width = read_u16(r);
    desc.height = read_u16(r);
    desc.width_high = read_u32(r);
    desc.width_tw = read_u32(r);
    desc.height_high = read_u32(r);
    desc.height_tw = read_u32(r);
    desc.flags = read_u8(r);
    return desc;
}


// bgra_to_argb / vica versa
void reverse_color_components(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto a = data[i + 0];
        const auto r = data[i + 1];
        const auto g = data[i + 2];
        const auto b = data[i + 3];
        data[i + 0] = b;
        data[i + 1] = g;
        data[i + 2] = r;
        data[i + 3] = a;
    }
}

// abgr_to_argb / vica versa
void convert_color_to_bgra(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto r = data[i + 0];
        const auto b = data[i + 2];
        data[i + 0] = b;
        data[i + 2] = r;
    }
}

// abgr_to_argb / vica versa
void convert_clut(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 4) {
        const auto r = data[i + 1];
        const auto b = data[i + 3];
        data[i + 1] = b;
        data[i + 3] = r;
    }
}

size_t ekUncompress(calo_reader_t* r, uint8_t* dest, size_t dest_size) {
    uint16_t chunk_size = read_u16(r);
    calo_writer_t w = new_writer(100);
    while (chunk_size) {
        write_span(&w, r->p, chunk_size);
        r->p += chunk_size;
        chunk_size = read_u16(r);
    }

    mz_ulong sz = dest_size;
    mz_uncompress(dest, &sz, w.data, (mz_ulong) (w.p - w.data));

    free_writer(&w);
    return sz;
}

void readBitmapARGB(calo_reader_t* r, bitmap_data_t* bitmap) {
    BitmapItemHeader desc = read_bitmap_item_header(r);
    const uint8_t compressed = read_u8(r);
    EK_ASSERT(desc.width_tw == desc.width * 20u);
    EK_ASSERT(desc.height_tw == desc.height * 20u);
    bitmap->width = desc.width;
    bitmap->height = desc.height;
    bitmap->bpp = desc.stride / bitmap->width;
    bitmap->alpha = desc.flags != 0;
    bitmap->data.resize(bitmap->width * bitmap->height * bitmap->bpp);
    const auto bm_size = bitmap->data.size();
    auto* bm_data = bitmap->data.data();
    if ((compressed & 1) != 0) {
        auto written = ekUncompress(r, bm_data, bm_size);
        if (written != bm_size) {
            log_error("bitmap decompress error");
        }
    } else {
        memcpy(bm_data, r->p, bm_size);
        r->p += bm_size;
    }

    reverse_color_components(bm_data, bm_size);
}

void readBitmapCLUT(calo_reader_t* r, bitmap_data_t* bitmap) {
    BitmapItemHeader desc = read_bitmap_item_header(r);
    EK_ASSERT(desc.width_tw == desc.width * 20u);
    EK_ASSERT(desc.height_tw == desc.height * 20u);
    uint8_t nColors = read_u8(r);
    if (nColors == 0) {
        nColors = 0xFF;
    }

    // read padding to align
    read_u16(r);

    uint32_t colorTable[256];
    for (int i = 0; i < nColors; ++i) {
        colorTable[i] = read_u32(r);
    }
    // convert color table to our cairo format
    convert_color_to_bgra(reinterpret_cast<uint8_t*>(colorTable), nColors * 4);

    if ((desc.flags & 0x1) != 0) {
        // transparent
        colorTable[0] = 0x0;
    }
    Array<uint8_t> buffer{};
    buffer.resize(desc.stride * desc.height);
    auto written = ekUncompress(r, buffer.data(), buffer.size());
    if (written != buffer.size()) {
        log_error("bitmap decompress error");
    }
    bitmap->width = desc.width;
    bitmap->height = desc.height;
    bitmap->bpp = 4;
    bitmap->alpha = desc.flags != 0;
    bitmap->data.resize(bitmap->width * bitmap->height * 4);
    auto* pixels = reinterpret_cast<uint32_t*>(bitmap->data.data());
    for (size_t i = 0; i < buffer.size(); ++i) {
        pixels[i] = colorTable[buffer[i]];
    }
}

void readBitmapJPEG(const uint8_t* data, int size, bitmap_data_t* bitmap) {
    int w = 0, h = 0, channels = 0;
    stbi_uc* imageData = stbi_load_from_memory(data, size,
                                               &w, &h, &channels, 4);

    if (imageData != nullptr) {
        bitmap->width = w;
        bitmap->height = h;
        bitmap->bpp = 4;
        bitmap->alpha = false;
        bitmap->data.resize(bitmap->width * bitmap->height * bitmap->bpp);
        memcpy(bitmap->data.data(), imageData, bitmap->data.size());
        free(imageData);
        convert_color_to_bgra(bitmap->data.data(), bitmap->data.size());
    }
}

bitmap_data_t* parse_bitmap_data(const uint8_t* data, uint32_t size) {
    bitmap_data_t* bitmap = new bitmap_data_t;

    calo_reader_t reader = {};
    reader.p = (uint8_t*) data;

    const uint16_t sig = read_u16(&reader);
    if (sig == SIGNATURE_ARGB) {
        readBitmapARGB(&reader, bitmap);
    } else if (sig == SIGNATURE_CLUT) {
        readBitmapCLUT(&reader, bitmap);
    } else if (sig == SIGNATURE_JPEG) {
        readBitmapJPEG(data, (int) size, bitmap);
    } else {
        log_error("unsupported dat");
    }

    return bitmap;
}
