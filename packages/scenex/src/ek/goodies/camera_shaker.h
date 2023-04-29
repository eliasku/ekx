#ifndef EKX_GOODIES_CAMERA_SHAKER_H
#define EKX_GOODIES_CAMERA_SHAKER_H

#include <ek/math.h>
#include <ekx/app/time_layers.h>
#include "ecx/ecx.h"

typedef struct camera_shaker {
    time_layer_t timer;
    float state;
    /**
     * max camera rotation delta
     */
    float rotation;
    /**
     * max camera offset delta
     */
    vec2_t offset;
    /**
     * max camera scale delta
     */
    vec2_t scale;
} camera_shaker_t;

#ifdef __cplusplus
extern "C" {
#endif

void setup_camera_shaker(void);
void camera_shaker_start(entity_t e, float v);
void camera_shaker_set(entity_t e);
void update_camera_shakers(void);

#ifdef __cplusplus
}
#endif

#endif // EKX_GOODIES_CAMERA_SHAKER_H
