#ifndef EK_FORMAT_SG_H
#define EK_FORMAT_SG_H

#include "dto.h"

#ifdef __cplusplus
extern "C" {
#endif

sg_keyframe_transform_t add_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b);

sg_keyframe_transform_t sub_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b);

sg_node_data_t sg_node_data_ctr(void);

model3d_t create_cube(vec3_t position, vec3_t size, color_t color);

model3d_t create_plane(vec3_t position, vec2_t size, color_t color);

#ifdef __cplusplus
}
#endif

#endif // EK_FORMAT_SG_H
