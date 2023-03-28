#pragma once

#include <ecx/ecx.hpp>
#include <ek/math.h>
#include "transform_api.h"

namespace ek {

// 32 bytes
struct WorldTransform2D {
    mat3x2_t matrix = mat3x2_identity(); // 4 * 6 = 24
    color2_t color = color2_identity(); // 2 * 4 = 8
};

// TODO: mat2x2, position,
struct Transform2D {
    // 24
    mat3x2_t matrix = mat3x2_identity();

    // 8
    color2_t color = color2_identity();

    // 16
    vec2_t cachedScale = vec2(1, 1);
    vec2_t cachedSkew = vec2(0, 0);

    inline void setX(float x) {
        matrix.tx = x;
    }

    inline void setY(float y) {
        matrix.ty = y;
    }

    inline void setMatrix(const mat3x2_t m) {
        matrix = m;
        cachedScale = mat2_get_scale(m.rot);
        cachedSkew = mat2_get_skew(m.rot);
    }

    [[nodiscard]]
    inline float getX() const {
        return matrix.tx;
    }

    [[nodiscard]]
    inline float getY() const {
        return matrix.ty;
    }

    inline void translate(float x, float y) {
        matrix.pos.x += x;
        matrix.pos.y += y;
    }

    inline void set_position(vec2_t position_) {
        matrix.pos = position_;
    }

    [[nodiscard]]
    inline vec2_t getPosition() const {
        return matrix.pos;
    }

    inline void set_position(float x, float y) {
        matrix.tx = x;
        matrix.ty = y;
    }

    inline void set_position(vec2_t position, vec2_t pivot) {
        const auto xx = -pivot.x;
        const auto yy = -pivot.y;
        matrix.tx = position.x + matrix.a * xx + matrix.c * yy;
        matrix.ty = position.y + matrix.d * yy + matrix.b * xx;
    }

    inline void set_position(vec2_t position_, vec2_t pivot_, vec2_t origin_) {
        const auto x = position_.x + origin_.x;
        const auto y = position_.y + origin_.y;
        const auto xx = -origin_.x - pivot_.x;
        const auto yy = -origin_.y - pivot_.y;
        matrix.tx = x + matrix.a * xx + matrix.c * yy;
        matrix.ty = y + matrix.d * yy + matrix.b * xx;
    }

    void scale(float factor) {
        cachedScale.x *= factor;
        cachedScale.y *= factor;
        updateMatrix2x2();
    }

    void set_scale(float value) {
        cachedScale.x = value;
        cachedScale.y = value;
        updateMatrix2x2();
    }

    void set_scale(vec2_t value) {
        cachedScale = value;
        updateMatrix2x2();
    }

    inline void setScaleX(float x) {
        cachedScale.x = x;
        updateMatrix2x2();
    }

    inline void setScaleY(float y) {
        cachedScale.y = y;
        updateMatrix2x2();
    }

    [[nodiscard]]
    inline vec2_t getScale() const {
        return cachedScale;
    }

    [[nodiscard]]
    inline float getScaleX() const {
        return cachedScale.x;
    }

    [[nodiscard]]
    inline float getScaleY() const {
        return cachedScale.y;
    }

    void set_rotation(float value) {
        cachedSkew.x = value;
        cachedSkew.y = value;
        const auto sn = sinf(value);
        const auto cs = cosf(value);
        matrix.a = cs * cachedScale.x;
        matrix.b = sn * cachedScale.x;
        matrix.c = -sn * cachedScale.y;
        matrix.d = cs * cachedScale.y;
    }

    [[nodiscard]]
    inline float getRotation() const {
//        return skew.x == skew.y ? skew.y : 0.0f;
        return cachedSkew.y;
    }

    [[nodiscard]]
    inline vec2_t getSkew() const {
        return cachedSkew;
    }

    inline void setSkew(vec2_t value) {
        cachedSkew = value;
        updateMatrix2x2();
    }

    inline void setTransform(vec2_t position, vec2_t scale_, vec2_t skew_, vec2_t pivot_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        set_position(position, pivot_);
    }

    void setTransform(vec2_t position_, vec2_t scale_, float rotation_) {
        cachedScale = scale_;
        cachedSkew.x = rotation_;
        cachedSkew.y = rotation_;
        const auto sn = sinf(rotation_);
        const auto cs = cosf(rotation_);
        matrix.a = cs * cachedScale.x;
        matrix.b = sn * cachedScale.x;
        matrix.c = -sn * cachedScale.y;
        matrix.d = cs * cachedScale.y;
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    inline void setTransform(vec2_t position_, vec2_t scale_, vec2_t skew_) {
        cachedScale = scale_;
        cachedSkew = skew_;
        updateMatrix2x2();
        matrix.tx = position_.x;
        matrix.ty = position_.y;
    }

    void rotate(float value) {
        cachedSkew.x += value;
        cachedSkew.y += value;
        updateMatrix2x2();
    }

    void translate(vec2_t delta) {
        set_position(getPosition() + delta);
    }

    void lerpScale(vec2_t target, float t) {
        set_scale(lerp_vec2(getScale(), target, t));
    }

    void lerpPosition(vec2_t target, float t) {
        set_position(lerp_vec2(getPosition(), target, t));
    }

    void lerpRotation(float target, float t) {
        set_rotation(lerp_f32(getRotation(), target, t));
    }

    inline void updateMatrix2x2() {
        matrix.a = cosf(cachedSkew.y) * cachedScale.x;
        matrix.b = sinf(cachedSkew.y) * cachedScale.x;
        matrix.c = -sinf(cachedSkew.x) * cachedScale.y;
        matrix.d = cosf(cachedSkew.x) * cachedScale.y;
    }
};

}

typedef ek::Transform2D transform2d_t;
typedef ek::WorldTransform2D world_transform2d_t;
