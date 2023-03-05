#ifndef EK_FORMAT_IMAGE_DATA_H
#define EK_FORMAT_IMAGE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum image_data_type {
    IMAGE_DATA_NORMAL = 0,
    IMAGE_DATA_CUBE_MAP = 1
} image_data_type;

typedef struct image_path_t {
    char str[128];
} image_path_t;

typedef struct image_data_t {
    uint32_t type;
    uint32_t format_mask;
    uint32_t images_num;
    image_path_t images[6];
} image_data_t;

#ifdef __cplusplus
}
#endif

#endif
