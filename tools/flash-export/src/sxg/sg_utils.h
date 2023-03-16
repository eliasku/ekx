#ifndef FLASH_EXPORT_UTILS_H
#define FLASH_EXPORT_UTILS_H

#include <gen_sg.h>

#ifdef __cplusplus
extern "C" {
#endif

sg_keyframe_transform_t add_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b);

sg_keyframe_transform_t sub_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b);

sg_node_data_t sg_node_data_ctr(void);

#ifdef __cplusplus
}
#endif

#endif // FLASH_EXPORT_UTILS_H
