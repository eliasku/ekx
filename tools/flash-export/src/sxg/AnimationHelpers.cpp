#include "AnimationHelpers.hpp"

#include <ek/format/sg.h>
#include "../xfl/Doc.hpp"
#include <ek/log.h>
#include <ek/assert.h>

namespace ek::xfl {

inline float sign(float a) {
    return a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
}

static sg_keyframe_transform_t createKeyFrameTransform(const Element& el) {
    const mat3x2_t m = el.transform.matrix;
    sg_keyframe_transform_t r;
    r.position = vec2_transform(el.transformationPoint, m);
    r.scale = mat3x2_get_scale(m);
    r.skew = mat3x2_get_skew(m);
    r.pivot = el.transformationPoint;
    r.color = el.transform.color;
    return r;
}

static void fixRotation(sg_keyframe_transform_t* curr, const sg_keyframe_transform_t* prev) {
    if (prev->skew.x + MATH_PI < curr->skew.x) {
        curr->skew.x -= MATH_TAU;
    } else if (prev->skew.x - MATH_PI > curr->skew.x) {
        curr->skew.x += MATH_TAU;
    }
    if (prev->skew.y + MATH_PI < curr->skew.y) {
        curr->skew.y -= MATH_TAU;
    } else if (prev->skew.y - MATH_PI > curr->skew.y) {
        curr->skew.y += MATH_TAU;
    }
}

static void addRotation(sg_keyframe_transform_t* curr, const Frame& frame, const sg_keyframe_transform_t* prev) {
    float additionalRotation = 0.0f;
    const RotationDirection rotate = frame.motionTweenRotate;
    const float times = (float)frame.motionTweenRotateTimes;
// If a direction is specified, take it into account
    if (rotate != RotationDirection::none) {
        float direction = (rotate == RotationDirection::cw ? 1.0f : -1.0f);
// negative scales affect rotation direction
        direction *= sign(curr->scale.x) * sign(curr->scale.y);

        while (direction < 0 && prev->skew.x < curr->skew.x) {
            curr->skew.x -= MATH_TAU;
        }
        while (direction > 0 && prev->skew.x > curr->skew.x) {
            curr->skew.x += MATH_TAU;
        }
        while (direction < 0 && prev->skew.y < curr->skew.y) {
            curr->skew.y -= MATH_TAU;
        }
        while (direction > 0 && prev->skew.y > curr->skew.y) {
            curr->skew.y += MATH_TAU;
        }

        // additional rotations specified?
        additionalRotation += times * 2.0f * MATH_PI * direction;
    }

    curr->skew.x += additionalRotation;
    curr->skew.y += additionalRotation;
}

sg_keyframe_transform_t extractTweenDelta(const Frame& frame, const Element& el0, const Element& el1) {
    auto t0 = createKeyFrameTransform(el0);
    auto t1 = createKeyFrameTransform(el1);
    fixRotation(&t1, &t0);
    addRotation(&t1, frame, &t0);
    return sub_keyframe_transform(&t1, &t0);
}

sg_movie_frame_t createFrameModel(const Frame& frame) {
    sg_movie_frame_t ef = {};
    ef.easing = nullptr;
    ef.visible = true;
    ef.index = frame.index;
    ef.duration = frame.duration;
    if (frame.tweenType == TweenType::classic) {
        ef.motion_type = 1;
        for (const auto& fd : frame.tweens) {
            sg_easing_t g = {};
            g.attribute = (uint8_t)fd.target;
            g.ease = (float)fd.intensity / 100.0f;
            g.curve = nullptr;
            arr_assign_((void**) &g.curve, sizeof(sg_easing_t), (void*) fd.custom_ease.data());
            arr_push(ef.easing, g);
        }
        if (!ef.easing) {
            sg_easing_t g = {};
            g.curve = nullptr;
            g.ease = (float)frame.acceleration / 100.0f;
            arr_push(ef.easing, g);
        }
        ef.rotate = (int32_t) frame.motionTweenRotate;
        ef.rotate_times = frame.motionTweenRotateTimes;
    } else if (frame.tweenType == TweenType::motion_object) {
        log_warn("motion object is not supported");
    }
    return ef;
}

void setupFrameFromElement(sg_movie_frame_t& target, const Element& el) {
    target.transform = createKeyFrameTransform(el);
    target.visible = el.isVisible;
    if (el.symbolType == SymbolType::graphic) {
        target.loop_mode = static_cast<int>(el.loop);
        target.first_frame = el.firstFrame;
    }
}

}
