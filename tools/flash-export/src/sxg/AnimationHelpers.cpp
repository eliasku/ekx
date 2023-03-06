#include "AnimationHelpers.hpp"

#include <ek/format/SGFile.hpp>
#include "../xfl/Doc.hpp"
#include <ek/log.h>
#include <ek/assert.h>

namespace ek::xfl {

inline float sign(float a) {
    return a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
}

static sg_keyframe_transform createKeyFrameTransform(const Element& el) {
    const mat3x2_t m = el.transform.matrix;
    sg_keyframe_transform r;
    r.position = vec2_transform(el.transformationPoint, m);
    r.scale = mat3x2_get_scale(m);
    r.skew = mat3x2_get_skew(m);
    r.pivot = el.transformationPoint;
    r.color = el.transform.color;
    return r;
}

static void fixRotation(sg_keyframe_transform* curr, const sg_keyframe_transform* prev) {
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

static void addRotation(sg_keyframe_transform* curr, const Frame& frame, const sg_keyframe_transform* prev) {
    float additionalRotation = 0.0f;
    const auto rotate = frame.motionTweenRotate;
    const auto times = frame.motionTweenRotateTimes;
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

sg_keyframe_transform extractTweenDelta(const Frame& frame, const Element& el0, const Element& el1) {
    auto t0 = createKeyFrameTransform(el0);
    auto t1 = createKeyFrameTransform(el1);
    fixRotation(&t1, &t0);
    addRotation(&t1, frame, &t0);
    return sub_keyframe_transform(&t1, &t0);
}

SGMovieFrameData createFrameModel(const Frame& frame) {
    SGMovieFrameData ef;
    ef.index = frame.index;
    ef.duration = frame.duration;
    if (frame.tweenType == TweenType::classic) {
        ef.motion_type = 1;
        for (const auto& fd : frame.tweens) {
            auto& g = ef.easing.emplace_back();
            g.attribute = static_cast<uint8_t>(fd.target);
            g.ease = static_cast<float>(fd.intensity) / 100.0f;
            g.curve = fd.custom_ease;
        }
        if (ef.easing.empty()) {
            auto& g = ef.easing.emplace_back();
            g.attribute = 0;
            g.ease = static_cast<float>(frame.acceleration) / 100.0f;
        }
        ef.rotate = (int) frame.motionTweenRotate;
        ef.rotateTimes = frame.motionTweenRotateTimes;
    } else if (frame.tweenType == TweenType::motion_object) {
        log_warn("motion object is not supported");
    }
    return ef;
}

void setupFrameFromElement(SGMovieFrameData& target, const Element& el) {
    target.transform = createKeyFrameTransform(el);
    target.visible = el.isVisible;
    if (el.symbolType == SymbolType::graphic) {
        target.loopMode = static_cast<int>(el.loop);
        target.firstFrame = el.firstFrame;
    }
}

}
