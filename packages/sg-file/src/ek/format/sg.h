#ifndef EK_FORMAT_SG_H
#define EK_FORMAT_SG_H

#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sg_filter_type {
    SG_FILTER_NONE,
    SG_FILTER_SHADOW,
    SG_FILTER_GLOW,
} sg_filter_type;

typedef struct sg_filter_t {
    sg_filter_type type:32;
    uint32_t quality;
    color_t color;
    vec2_t blur;
    vec2_t offset;
} sg_filter_t;

typedef struct sg_text_layer {
    color_t color;
    vec2_t offset;
    float blur_radius;
    int blur_iterations;
    int strength;
}sg_text_layer;

//struct SGDynamicTextData {
//    String text;
//    string_hash_t font;
//    float size;
//    vec2_t alignment = {};
//    rect_t rect = {};
//    float lineSpacing = 0.0f;
//    float lineHeight = 0.0f;
//
//    PodArray<sg_text_layer> layers;
//
//    bool wordWrap = false;
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        io(text, font, size, alignment, rect, lineSpacing, lineHeight, layers, wordWrap);
//    }
//};

// TODO: it should be optimized with easing table store :)
//struct SGEasingData {
//    uint8_t attribute = 0;
//    float ease = 0.0f;
//    PodArray<vec2_t> curve{};
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        io(attribute, ease, curve);
//#ifndef NDEBUG
//        EK_ASSERT(curve.size() <= 8);
//#endif
//    }
//};

typedef struct sg_keyframe_transform {
    vec2_t position;
    vec2_t scale;
    vec2_t skew;
    vec2_t pivot;
    color2f_t color;
} sg_keyframe_transform;

inline sg_keyframe_transform add_keyframe_transform(const sg_keyframe_transform* a, const sg_keyframe_transform* b) {
    return (sg_keyframe_transform){
            add_vec2(a->position, b->position),
            add_vec2(a->scale, b->scale),
            add_vec2(a->skew, b->skew),
            add_vec2(a->pivot, b->pivot),
            add_color2f(a->color, b->color),
    };
}

inline sg_keyframe_transform sub_keyframe_transform(const sg_keyframe_transform* a, const sg_keyframe_transform* b) {
    return (sg_keyframe_transform){
            sub_vec2(a->position, b->position),
            sub_vec2(a->scale, b->scale),
            sub_vec2(a->skew, b->skew),
            sub_vec2(a->pivot, b->pivot),
            sub_color2f(a->color, b->color),
    };
}

//
//struct SGMovieFrameData {
//    int index = 0;
//    int duration = 0;
//    int motion_type = 0;
//
//    Array<SGEasingData> easing{};
//
//    SGKeyFrameTransform transform;
//
//    bool visible = true;
//
//    // graphic frame control
//    int loopMode = 0; // loop_mode
//    int firstFrame = 0;
//
//    // rotation postprocessing
//
//    int rotate = 0; // rotate_direction
//    int rotateTimes = 0;
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        io(index, duration, motion_type, transform, easing,
//           loopMode, firstFrame, rotate, rotateTimes, visible);
//
//#ifndef NDEBUG
//        uint32_t mask = 0;
//        for(auto& ease : easing) {
//            EK_ASSERT((mask & ease.attribute) == 0);
//            mask |= 1 << ease.attribute;
//        }
//#endif
//    }
//
//    [[nodiscard]]
//    float getLocalTime(float time) const {
//        return (time - static_cast<float>(index)) / static_cast<float>(duration);
//    }
//};
//
//struct SGNodeData;
//
//struct SGMovieLayerData {
//    Array<SGMovieFrameData> frames;
//
//    // temp for restoring target ID
//    PodArray<SGNodeData*> targets;
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        io(frames);
//    }
//};

//typedef struct sg_frame_label {
//    String name;
//    int frame;
//} sg_frame_label;
//
//typedef struct sg_frame_script {
//    String code;
//    int frame;
//} sg_frame_script;

//struct SGMovieData {
//    int frames = 1;
//    float fps = 24.0f;
//    Array<SGMovieLayerData> layers;
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        // TODO: labels and scripts
//        io(frames, fps, layers);
//    }
//};
//
//struct SGNodeData {
//
//    mat3x2_t matrix = mat3x2_identity();
//    color2f_t color = color2f();
//
//    // instance name
//    string_hash_t name;
//
//    // name in library
//    string_hash_t libraryName;
//
//    // sprite id
//    string_hash_t sprite;
//
//    bool button = false;
//    bool touchable = true;
//    bool visible = true;
//    bool scissorsEnabled = false;
//    bool hitAreaEnabled = false;
//    bool boundsEnabled = false;
//    rect_t boundingRect;
//    rect_t scaleGrid;
//    Array<SGNodeData> children;
//    Array<SGDynamicTextData> dynamicText;
//    Array<SGMovieData> movie;
//    Array<SGFrameLabel> labels;
//    Array<SGFrameScript> scripts;
//    int32_t movieTargetId = -1;
//
//    template<typename S>
//    void serialize(IO<S>& io) {
//        io(
//                name,
//                libraryName,
//                matrix,
//                sprite,
//
//                button,
//                touchable,
//                visible,
//                scissorsEnabled,
//                hitAreaEnabled,
//                boundsEnabled,
//
//                boundingRect,
//                scaleGrid,
//
//                color,
//
//                children,
//                dynamicText,
//                movie,
//
//                movieTargetId
//        );
//    }
//};

typedef struct sg_scene_info {
    // public export linkage name
    string_hash_t name;
    // internal symbol name
    string_hash_t linkage;
} sg_scene_info;

#ifdef __cplusplus
}
#endif

#endif // EK_FORMAT_SG_H