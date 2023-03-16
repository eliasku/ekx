#include "sg_utils.h"

sg_keyframe_transform_t add_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b) {
    return (sg_keyframe_transform_t){
            add_vec2(a->position, b->position),
            add_vec2(a->scale, b->scale),
            add_vec2(a->skew, b->skew),
            add_vec2(a->pivot, b->pivot),
            add_color2f(a->color, b->color),
    };
}

sg_keyframe_transform_t sub_keyframe_transform(const sg_keyframe_transform_t* a, const sg_keyframe_transform_t* b) {
    return (sg_keyframe_transform_t){
            sub_vec2(a->position, b->position),
            sub_vec2(a->scale, b->scale),
            sub_vec2(a->skew, b->skew),
            sub_vec2(a->pivot, b->pivot),
            sub_color2f(a->color, b->color),
    };
}

sg_node_data_t sg_node_data_ctr(void) {
    sg_node_data_t data = {0};
    data.matrix = mat3x2_identity();
    data.color = color2f();
    data.flags = SG_NODE_TOUCHABLE | SG_NODE_VISIBLE;
    data.movie_target_id = -1;
    return data;
}
