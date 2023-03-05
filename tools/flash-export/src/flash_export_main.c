#include <ek/log.h>
#include "image_set.h"

#include <stb/stb_image_write.h>

char* freadline(FILE* f, char* buf, uint32_t max) {
    char* res = fgets(buf, (int)max, f);
    if (res) {
        const uint32_t len = strlen(res);
        if (len && res[len - 1] == '\n') {
            res[len - 1] = 0;
        }
        return res;
    }
    buf[0] = 0;
    return 0;
}

int exportFlash(const char* config_path) {
    FILE* f = fopen(config_path, "rb");
    if (!f) {
        log_error("error parse config %s", config_path);
        return 1;
    }

    // path <path>
    // output_path <output>
    // output_images_path <outputImages>
    // resoulutions_num %u
    // resolution_scale %f
    // ...

    char flash_path[1024];
    char output_path[1024];
    char output_images_path[1024];
    freadline(f, flash_path, sizeof flash_path);
    freadline(f, output_path, sizeof output_path);
    freadline(f, output_images_path, sizeof output_images_path);

    uint32_t resolutions_num = 0;
    fscanf(f, "%u\n", &resolutions_num);
    image_set_t image_set = {0};
    image_set.resolutions_num = resolutions_num;
    for (uint32_t i = 0; i < resolutions_num; ++i) {
        float scale;
        fscanf(f, "%f\n", &scale);
        image_set.resolutions[i].index = i;
        image_set.resolutions[i].scale = scale;
    }

    export_xfl(flash_path, &image_set, output_path, output_images_path);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }
    log_init();

    if (strcmp("render", argv[1]) == 0) {
        runFlashFilePrerender(argc - 2, argv + 2);
    } else if (strcmp("export", argv[1]) == 0) {
        if (argc > 2) {
            return exportFlash(argv[2]);
        } else {
            return 1;
        }
    }
    return 0;
}
