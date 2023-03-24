#include <ek/texture_loader.h>
#include <ek/bitmap.h>
#include <ek/app.h>
#include <ek/log.h>
#include <stdlib.h>
#include <stdio.h>

static int ek_texture_loader__load_bytes(const char* base_path, const char* url, void** out_buffer, uint32_t* out_size) {
    char path_buffer[1024];
    path_buffer[0] = 0;

    if (base_path && base_path[0]) {
        const size_t len = strlen(base_path);
        strcat(path_buffer, base_path);
        if (len > 0 && path_buffer[len - 1] != '/') {
            path_buffer[len] = '/';
            path_buffer[len + 1] = 0;
        }
    }
    strcat(path_buffer, url);
#if defined(__APPLE__)
    char file_path_buffer[1024];
    const char* file_path = ek_app_ns_bundle_path(path_buffer, file_path_buffer, sizeof(file_path_buffer));
#else
    const char* file_path = pathBuffer;
#endif

    FILE* stream = fopen(file_path, "rb");
    if (!stream) {
        return 1;
    }
    fseek(stream, 0, SEEK_END);
    const uint32_t size = (uint32_t) ftell(stream);
    *out_size = size;
    if (size) {
        void* buffer = malloc(size);
        if (buffer) {
            fseek(stream, 0, SEEK_SET);

            fread(buffer, size, 1u, stream);

            bool success = ferror(stream) == 0;
            fclose(stream);

            if (success) {
                *out_buffer = buffer;
                return 0;
            }
            free(buffer);
            *out_buffer = NULL;
        }
        *out_size = 0;
    }
    return 2;
}

static void ek_texture_loader__setup_image_desc(const ek_texture_loader_image_data* data, sg_image_desc* desc) {
    EK_ASSERT(data != 0);
    EK_ASSERT(desc != 0);
    //desc->label = label;
    desc->type = data->isCubeMap ? SG_IMAGETYPE_CUBE : SG_IMAGETYPE_2D;
    desc->width = data->width;
    desc->height = data->height;
    desc->usage = SG_USAGE_IMMUTABLE;
    desc->pixel_format = SG_PIXELFORMAT_RGBA8;
    desc->min_filter = SG_FILTER_LINEAR;
    desc->mag_filter = SG_FILTER_LINEAR;
    desc->wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    desc->wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    for (int i = 0; i < data->subImagesCount; ++i) {
        const ek_texture_loader_sub_image* sub_image = data->subImages + i;
        desc->data.subimage[i][0].ptr = sub_image->data;
        desc->data.subimage[i][0].size = sub_image->height * sub_image->stride;
    }
}

void ek_texture_loader_load(ek_texture_loader* loader) {
    loader->loading = true;
    loader->status = 0;
    loader->progress = 0.0f;
    loader->imageData.isCubeMap = loader->isCubeMap;
    loader->imageData.subImagesCount = loader->imagesToLoad;
}

void ek_texture_loader_update(ek_texture_loader* loader) {
    if (loader->imagesToLoad <= 0 || !loader->loading) {
        return;
    }
    if (loader->imagesStartLoading < loader->imagesToLoad) {
        const int idx = loader->imagesStartLoading++;
        log_debug("poll loading image #%d / %d", idx, loader->imagesToLoad);
        void* buffer = NULL;
        uint32_t buffer_size = 0;
        int result = ek_texture_loader__load_bytes(loader->basePath.path, loader->urls[idx].path, &buffer, &buffer_size);
        if (result == 0) {
            ++loader->imagesLoaded;
            loader->progress = (float) loader->imagesLoaded / (float) loader->imagesToLoad;
            bitmap_t bitmap = {0};
            bitmap_decode(&bitmap, buffer, buffer_size, loader->premultiplyAlpha);
            if (bitmap.pixels) {
                loader->imageData.subImages[idx].data = bitmap.pixels;
                loader->imageData.subImages[idx].width = (int) bitmap.w;
                loader->imageData.subImages[idx].height = (int) bitmap.h;
                loader->imageData.subImages[idx].stride = (int) (bitmap.w * 4u);
            }
            log_debug("texture #%d loaded %d of %d", idx, loader->imagesLoaded, loader->imagesToLoad);
        } else {
            EK_ASSERT(false);
        }
    } else if (loader->imagesLoaded == loader->imagesToLoad) {
        loader->imageData.width = loader->imageData.subImages[0].width;
        loader->imageData.height = loader->imageData.subImages[0].height;
        ek_texture_loader__setup_image_desc(&loader->imageData, &loader->desc);
        loader->image = sg_make_image(&loader->desc);
        for (int i = 0; i < EK_TEXTURE_LOADER_IMAGES_MAX_COUNT; ++i) {
            const void* subImageData = loader->imageData.subImages[i].data;
            free((void*) subImageData);
            memset(&loader->imageData.subImages[i], 0, sizeof(ek_texture_loader_sub_image));
        }
        loader->progress = 1.0f;
        loader->loading = false;
    }
}