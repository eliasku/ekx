#ifndef SCE_TIME_LAYERS_H
#define SCE_TIME_LAYERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct time_layer_state_t {
    float scale;
    float dt;
    float total;
    float pad_;
} time_layer_state_t;

enum {
    TIME_LAYER_ROOT = 0,
    TIME_LAYER_GAME = 1,
    TIME_LAYER_HUD = 2,
    TIME_LAYER_UI = 3,
    TIME_LAYER_MAX_COUNT = 4,
};

typedef uint8_t time_layer_t;

extern float g_time_dt;
extern time_layer_state_t g_time_layers[TIME_LAYER_MAX_COUNT];

void init_time_layers(void);
void update_time_layers(float raw_dt);

#ifdef __cplusplus
}
#endif

#endif // SCE_TIME_LAYERS_H
