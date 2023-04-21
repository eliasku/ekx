#include "time_layers.h"

#include <ek/assert.h>

float g_time_dt;
time_layer_state_t g_time_layers[TIME_LAYER_MAX_COUNT];

#define TIME_LAYER_DELTA_TIME_MAX (0.3f)

time_layer_state_t time_layer(time_layer_t tl) {
    EK_ASSERT(tl < TIME_LAYER_MAX_COUNT);
    return g_time_layers[tl];
}

static float update_time_layer(time_layer_state_t* layer, float dt) {
    const float dt1 = dt * layer->scale;
    layer->dt = dt1;
    layer->total += dt1;
    return dt1;
}

void init_time_layers(void) {
    for (uint32_t i = 0; i < TIME_LAYER_MAX_COUNT; ++i) {
        g_time_layers[i].scale = 1.0f;
    }
}

void update_time_layers(float dt) {
    g_time_dt = dt;
    if (dt > TIME_LAYER_DELTA_TIME_MAX) {
        dt = TIME_LAYER_DELTA_TIME_MAX;
    }
    dt = update_time_layer(g_time_layers, dt);
    update_time_layer(g_time_layers + 1, dt);
    update_time_layer(g_time_layers + 2, dt);
    update_time_layer(g_time_layers + 3, dt);
}
