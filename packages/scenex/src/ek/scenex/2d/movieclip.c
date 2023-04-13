#include "movieclip.h"
#include "transform2d.h"

#include <ek/buf.h>
#include <ek/scenex/base/node.h>

ecx_component_type MovieClip;
ecx_component_type MovieClipTarget;

static void MovieClip_ctor(component_handle_t i) {
    ((movieclip_t*) MovieClip.data[0])[i] = (movieclip_t) {
            .data = NULL,
            .time = 0.0f,
            .fps = 24.0f,
            .timer = 0,
            .playing = true,
    };
}

void MovieClip_setup(void) {
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "MovieClip", 16, 1, {sizeof(movieclip_t)}
        };
        init_component_type(&MovieClip, decl);
        MovieClip.ctor = MovieClip_ctor;
    }
    {
        const ecx_component_type_decl decl = (ecx_component_type_decl) {
                "MovieClipTarget", 16, 1, {sizeof(movieclip_target_index_t)}
        };
        init_component_type(&MovieClipTarget, decl);
    }
}

static void trunc_time(movieclip_t* mc) {
    if (mc->data) {
        const float max = (float) mc->data->frames;
        if (mc->time >= max) {
            mc->time -= max * truncf(mc->time / max);
        }
        if (mc->time < 0) {
            mc->time = 0;
        }
    }
}

static float ease(float x, const sg_easing_t* data);

static void apply_frame(entity_t e, movieclip_t* mc);

void MovieClip_update(void) {
    for (uint32_t i = 1; i < MovieClip.size; ++i) {
        movieclip_t* mc = (movieclip_t*) MovieClip.data[0] + i;
        const float dt = g_time_layers[mc->timer].dt;
        if (mc->playing) {
            mc->time += dt * mc->fps;
            trunc_time(mc);
            apply_frame(get_entity(&MovieClip, i), mc);
        }
    }
}

static int find_keyframe(const sg_movie_frame_t* frames, float t) {
    const uint32_t end = arr_size(frames);
    for (uint32_t i = 0; i < end; ++i) {
        const sg_movie_frame_t kf = frames[i];
        if (t >= (float) kf.index && t < (float) (kf.index + kf.duration)) {
            return (int) i;
        }
    }
    return -1;
}

typedef struct {
    float position;
    float scale;
    float skew;
    float color;
} easing_progress_t;

static easing_progress_t easing_progress(float t) {
    return (easing_progress_t) {t, t, t, t};
}

static float get_frame_local_time(const sg_movie_frame_t* frame, float time) {
    return (time - (float) frame->index) / (float) frame->duration;
}

static sg_keyframe_transform_t lerp(const sg_keyframe_transform_t* begin,
                                    const sg_keyframe_transform_t* end,
                                    const easing_progress_t* progress) {
    return (sg_keyframe_transform_t) {
            lerp_vec2(begin->position, end->position, progress->position),
            lerp_vec2(begin->scale, end->scale, progress->scale),
            lerp_vec2(begin->skew, end->skew, progress->skew),
            begin->pivot,
            lerp_color2f(begin->color, end->color, progress->color)
    };
}

static easing_progress_t get_easing_progress(const float t, const sg_easing_t* easing) {
    easing_progress_t progress = easing_progress(t);
    uint32_t easing_count = arr_size(easing);
    if (arr_empty(easing)) {
        return progress;
    }

    for (uint32_t i = 0; i < easing_count; ++i) {
        const sg_easing_t* e = easing + i;
        const float x = ease(t, e);
        if (e->attribute == 0) {
            progress = easing_progress(x);
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

static void apply_transform(entity_t e, const sg_keyframe_transform_t* keyframe) {
    transform2d_t* transform = add_transform2d(e);
    transform2d_set_transform_ex(transform, keyframe->position, keyframe->scale, keyframe->skew, keyframe->pivot);
    transform->color.scale = color_vec4(keyframe->color.scale);
    transform->color.offset = color_vec4(keyframe->color.offset);
}

static void update_target(float time, entity_t e, const sg_movie_layer_t* layer) {
    const int ki = find_keyframe(layer->frames, time);
    if (ki < 0) {
        set_visible(e, false);
        return;
    }
    const sg_movie_frame_t k1 = layer->frames[ki];
    set_visible(e, k1.visible);
    if (k1.motion_type == 1 && (uint32_t) ki < arr_size(layer->frames) - 1) {
        const sg_movie_frame_t k2 = layer->frames[ki + 1];
        const float t = get_frame_local_time(&k1, time);
        const easing_progress_t progress = get_easing_progress(t, k1.easing);
        const sg_keyframe_transform_t keyframe = lerp(&k1.transform, &k2.transform, &progress);
        apply_transform(e, &keyframe);
    } else {
        apply_transform(e, &k1.transform);
    }

    if (k1.loop_mode != 0) {
        const movieclip_t* mc = get_movieclip(e);
        if (mc) {
            const int32_t loop = k1.loop_mode;
            if (loop == 1) {
                goto_and_stop(e, time - (float) k1.index);
            } else if (loop == 2) {
                const float offset = fminf(time, (float) (k1.index + k1.duration)) - k1.index;
                float t = (float) k1.first_frame + offset;
                const sg_movie_t* mc_data = mc->data;
                if (mc_data && t > (float) mc_data->frames) {
                    t = (float) mc_data->frames;
                }
                goto_and_stop(e, t);
            } else if (loop == 3) {
                goto_and_stop(e, (float) k1.first_frame);
            }
        }
    }
}

static void apply_frame(entity_t e, movieclip_t* mc) {
    const sg_movie_t* data = mc->data;
    float time = mc->time;
    if (!data) {
        // no data - exit early
        return;
    }
    entity_t it = get_first_child(e);
    const int total_targets = (int) arr_size(data->layers);
    while (it.id) {
        const movieclip_target_index_t* ti = get_movieclip_target(it);
        if (ti) {
            const int idx = ti->key;
            if (idx < total_targets) {
                update_target(time, it, data->layers + idx);
            }
        }
        it = get_next_child(it);
    }
}

void goto_and_stop(entity_t e, float frame) {
    movieclip_t* mc = get_movieclip(e);
    mc->playing = false;
    mc->time = frame;
    trunc_time(mc);
    apply_frame(e, mc);
}


/*** EASING CALCULATION ***/

// math is not hard, but code has been stolen from precious web,
// look for `fl.motion`, BezierEase, BezierSegment, CustomEase

static int get_quadratic_roots(float out_roots[2], float a, float b, float c) {
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

static int get_cubic_roots(float out_roots[3], float a, float b, float c, float d) {
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

static float get_bezier_value_normalized(float t, float a, float b, float c, float d) {
    return (t * t * (d - a) + 3 * (1 - t) * (t * (c - a) + (1 - t) * (b - a))) * t + a;
}

static float get_bezier_y(const vec2_t* curve, float x) {
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

static float ease(float x, const sg_easing_t* data) {
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
