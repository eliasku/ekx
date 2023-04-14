#include "particle_decl.h"

rand_color_t rand_color(color_t color) {
    rand_color_t r = INIT_ZERO;
    r.colors[0] = color;
    r.colors_num = 1;
    return r;
}

rand_color_t rand_color_gradient(color_t color1, color_t color2) {
    rand_color_t r = INIT_ZERO;
    r.colors[0] = color1;
    r.colors[1] = color2;
    r.colors_num = 2;
    r.mode = RAND_COLOR_RAND_LERP;
    return r;
}

rand_color_t rand_color_steps(color_t color1, color_t color2) {
    rand_color_t r = INIT_ZERO;
    r.colors[0] = color1;
    r.colors[1] = color2;
    r.colors_num = 2;
    r.mode = RAND_COLOR_CONTINUOUS;
    return r;
}

color_t rand_color_next(rand_color_t* color) {
    ++color->state;
    uint32_t size = color->colors_num;
    if (!size) {
        return COLOR_WHITE;
    }
    if (size == 1) {
        return color->colors[0];
    }
    switch (color->mode) {
        case RAND_COLOR_RAND_LERP: {
            float t = random_f();
            uint32_t i = random_n(size - 1);
            return lerp_color(color->colors[i], color->colors[i + 1], t);
        }
        case RAND_COLOR_RAND_ELEMENT:
            return color->colors[random_n(color->colors_num)];
        case RAND_COLOR_CONTINUOUS:
            return color->colors[color->state % color->colors_num];
    }
    return COLOR_WHITE;
}

float_range_t float_range_zero(void) {
    float_range_t r;
    r.min = 0;
    r.max = 0;
    return r;
}

float_range_t float_range_val(float val) {
    float_range_t r;
    r.min = val;
    r.max = val;
    return r;
}
float_range_t float_range(float min, float max) {
    float_range_t r;
    r.min = min;
    r.max = max;
    return r;
}

float random_float_range(float_range_t range) {
    return random_range_f(range.min, range.max);
}


struct res_particle res_particle;

void setup_res_particle(void) {
    struct res_particle* R = &res_particle;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

particle_decl_t particle_decl(void) {
    particle_decl_t decl = INIT_ZERO;
    decl.alpha_start = float_range_val(1);
    decl.life_time = float_range_val(1);
    decl.acc_x_phase = float_range_val(0.5f * MATH_PI);
    decl.scale_start = float_range_val(1);
    return decl;
}

emitter_data_t emitter_data(void) {
    emitter_data_t r = INIT_ZERO;
    r.interval = 0.5f;
    r.burst = 1;
    r.burst_rotation_delta = float_range(1, 1.5f);
    r.speed = float_range(10, 100);
    r.acc = float_range_val(0);
    r.dir = float_range(0, MATH_TAU);
    return r;
}

