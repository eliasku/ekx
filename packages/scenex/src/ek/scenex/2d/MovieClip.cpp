#include "MovieClip.hpp"
#include "Transform2D.hpp"

#include <ek/scenex/base/node.h>

namespace ek {

float ease(float x, const sg_easing_t* data);

void apply_frame(entity_t e, MovieClip& mov);

void MovieClip::updateAll() {
    for (auto e: ecs::view<MovieClip>()) {
        auto& mov = ecs::get<MovieClip>(e);
        auto dt = g_time_layers[mov.timer].dt;
        if (mov.playing) {
            mov.time += dt * mov.fps;
            mov.trunc_time();
            apply_frame(e, mov);
        }
    }
}

int findKeyFrame(const sg_movie_frame_t* frames, float t) {
    const uint32_t end = arr_size(frames);
    for (int i = 0; i < end; ++i) {
        const sg_movie_frame_t kf = frames[i];
        if (t >= (float)kf.index && t < (float)(kf.index + kf.duration)) {
            return i;
        }
    }
    return -1;
}

struct easing_progress_t {
    float position;
    float scale;
    float skew;
    float color;

    void fill(float t) {
        position = scale = skew = color = t;
    }
};

float get_frame_local_time(const sg_movie_frame_t* frame, float time) {
    return (time - (float) frame->index) / (float) frame->duration;
}

sg_keyframe_transform_t lerp(const sg_keyframe_transform_t& begin,
                             const sg_keyframe_transform_t& end,
                             const easing_progress_t& progress) {
    return {
            lerp_vec2(begin.position, end.position, progress.position),
            lerp_vec2(begin.scale, end.scale, progress.scale),
            lerp_vec2(begin.skew, end.skew, progress.skew),
            begin.pivot,
            lerp_color2f(begin.color, end.color, progress.color)
    };
}

easing_progress_t get_easing_progress(const float t, const sg_easing_t* easing) {
    easing_progress_t progress{};
    uint32_t easing_count = arr_size(easing);
    if (arr_empty(easing)) {
        progress.fill(t);
        return progress;
    }

    for (uint32_t i = 0; i < easing_count; ++i) {
        const sg_easing_t* e = easing + i;
        const float x = ease(t, e);
        if (e->attribute == 0) {
            progress.fill(x);
            break;
        } else if (e->attribute == 1) {
            progress.position = x;
        } else if (e->attribute == 2) {
            progress.skew = x;
        } else if (e->attribute == 3) {
            progress.scale = x;
        } else if (e->attribute == 4) {
            progress.color = x;
        }
    }
    return progress;
}

void apply_transform(entity_t e, const sg_keyframe_transform_t& keyframe) {
    auto& transform = ecs::add<Transform2D>(e);
    transform.setTransform(keyframe.position, keyframe.scale, keyframe.skew, keyframe.pivot);
    transform.color.scale = color_vec4(keyframe.color.scale);
    transform.color.offset = color_vec4(keyframe.color.offset);
}

void update_target(float time, entity_t e, const sg_movie_layer_t & layer) {
    const auto ki = findKeyFrame(layer.frames, time);
    if (ki < 0) {
        set_visible(e, false);
        return;
    }
    const auto& k1 = layer.frames[ki];
    set_visible(e, k1.visible);
    if (k1.motion_type == 1 && (ki + 1) < arr_size(layer.frames)) {
        const auto& k2 = layer.frames[ki + 1];
        const float t = get_frame_local_time(&k1, time);
        const auto progress = get_easing_progress(t, k1.easing);
        const auto keyframe = lerp(k1.transform, k2.transform, progress);
        apply_transform(e, keyframe);
    } else {
        apply_transform(e, k1.transform);
    }

    if (k1.loop_mode != 0 && ecs::has<MovieClip>(e)) {
        auto& mc = ecs::get<MovieClip>(e);
        const auto loop = k1.loop_mode;
        if (loop == 1) {
            goto_and_stop(e, time - k1.index);
        } else if (loop == 2) {
            const auto offset = fmin(time, k1.index + k1.duration) - k1.index;
            auto t = k1.first_frame + offset;
            const auto* mcData = mc.data;
            if (mcData && t > mcData->frames) {
                t = mcData->frames;
            }
            goto_and_stop(e, t);
        } else if (loop == 3) {
            goto_and_stop(e, k1.first_frame);
        }
    }
}

void apply_frame(entity_t e, MovieClip& mov) {
    auto* data = mov.data;
    auto time = mov.time;
    if (!data) {
        // no data - exit early
        return;
    }
    auto it = get_first_child(e);
    const int total_targets = (int) arr_size(data->layers);
    while (it.id) {
        MovieClipTargetIndex* ti = ecs::try_get<MovieClipTargetIndex>(it);
        if (ti) {
            const int idx = ti->key;
            if (idx < total_targets) {
                update_target(time, it, data->layers[idx]);
            }
        }
        it = get_next_child(it);
    }
}

void goto_and_stop(entity_t e, float frame) {
    auto& mov = ecs::get<MovieClip>(e);
    mov.playing = false;
    mov.time = frame;
    mov.trunc_time();
    apply_frame(e, mov);
}


/*** EASING CALCULATION ***/

// math is not hard, but code has been stolen from precious web,
// look for `fl.motion`, BezierEase, BezierSegment, CustomEase

int get_quadratic_roots(float out_roots[2], float a, float b, float c) {
// make sure we have a quadratic
    if (a == 0.0f) {
        if (b == 0.0f) {
            return 0;
        }
        out_roots[0] = -c / b;
        return 1;
    }

    const float q = b * b - 4 * a * c;
    if (q > 0.0f) {
        const float x = sqrtf(q) / (2 * a);
        const float d = -b / (2 * a);
        out_roots[0] = d - x;
        out_roots[1] = d + x;
        return 2;
    } else if (q < 0.0f) {
        return 0;
    }
    out_roots[0] = -b / (2 * a);
    return 1;
}

int get_cubic_roots(float out_roots[3], float a = 0.0f, float b = 0.0f, float c = 0.0f, float d = 0.0f) {
    // make sure we really have a cubic
    if (a == 0.0f) {
        return get_quadratic_roots(out_roots, b, c, d);
    }

    // normalize the coefficients so the cubed term is 1 and we can ignore it hereafter
    b /= a;
    c /= a;
    d /= a;

    const float q = (b * b - 3 * c) / 9;               // won't change over course of curve
    const float q_cubed = q * q * q;                  // won't change over course of curve
    const float r = (2 * b * b * b - 9 * b * c + 27 * d) / 54; // will change because d changes
    // but parts with b and c won't change
    // determine if there are 1 or 3 real roots using r and q
    const float diff = q_cubed - r * r;
    if (diff >= 0.0f) {
        // avoid division by zero
        if (q == 0.0f) {
            out_roots[0] = 0.0f;
            return 1;
        }

        // three real roots
        const float theta = acosf(r / sqrtf(q_cubed)); // will change because r changes
        const float q_sqrt = sqrtf(q); // won't change

        out_roots[0] = -2 * q_sqrt * cosf(theta / 3.0f) - b / 3.0f;
        out_roots[1] = -2 * q_sqrt * cosf((theta + MATH_TAU) / 3.0f) - b / 3.0f;
        out_roots[2] = -2 * q_sqrt * cosf((theta + 4 * MATH_PI) / 3.0f) - b / 3.0f;

        return 3;
    }
    // one real root
    const float tmp = powf(sqrtf(-diff) + fabsf(r), 1.0f / 3.0f);
    const float r_sign = r > 0.0f ? 1.0f : (r < 0.0f ? -1.0f : 0.0f);
    out_roots[0] = -r_sign * (tmp + q / tmp) - b / 3;
    return 1;
}

float get_bezier_value_normalized(float t, float a, float b, float c, float d) {
    return (t * t * (d - a) + 3 * (1 - t) * (t * (c - a) + (1 - t) * (b - a))) * t + a;
}

float get_bezier_y(const vec2_t* curve, float x) {
    const float eps = 0.000001f;
    const float a = curve[0].x;
    const float b = curve[1].x;
    const float c = curve[2].x;
    const float d = curve[3].x;

    if (a < d) {
        if (x <= a + eps) return curve[0].y;
        if (x >= d - eps) return curve[3].y;
    } else {
        if (x >= a + eps) return curve[0].y;
        if (x <= d - eps) return curve[3].y;
    }

    const float coeffs[4] = {
            -a + 3 * b - 3 * c + d,
            3 * a - 6 * b + 3 * c,
            -3 * a + 3 * b,
            a
    };

    // x(t) = a*t^3 + b*t^2 + c*t + d
    float roots[3];
    int roots_count = get_cubic_roots(roots, coeffs[0], coeffs[1], coeffs[2], coeffs[3] - x);
    float time = 0.0f;
    for (int i = 0; i < roots_count; ++i) {
        const float r = roots[i];
        if (0.0f <= r && r <= 1.0f) {
            time = r;
            break;
        }
    }

    return get_bezier_value_normalized(time,
                                       curve[0].y,
                                       curve[1].y,
                                       curve[2].y,
                                       curve[3].y);
}

float ease(float x, const sg_easing_t* data) {
    float y = x;
    const uint32_t curve_len = arr_size(data->curve);
    if (curve_len > 3) {
        // bezier
        for (uint32_t i = 0; i < curve_len - 3; ++i) {
            const float x0 = data->curve[i].x;
            const float x1 = data->curve[i + 3].x;
            if (x0 <= x && x <= x1) {
                return get_bezier_y(data->curve + i, x);
            }
        }
    } else if (data->ease != 0) {
        float e = data->ease; //  / 100.0f
        float t;
        if (e < 0.0f) {
            // Ease in
            float inv = 1.0f - x;
            t = 1.0f - inv * inv;
            e = -e;
        } else {
            // Ease out
            t = x * x;
        }
        y = e * t + (1.0f - e) * x;
    }
    return y;
}

}
