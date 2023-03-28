#ifndef SCENEX_BASE_TWEEN_H
#define SCENEX_BASE_TWEEN_H

#include <ecx/ecx.h>
#include <ekx/app/time_layers.h>

typedef struct {
    void (* advanced)(entity_t e, float t);

    void (* completed)(entity_t e);

    int data[4];
    float delay;
    float time;
    float duration;
    TimeLayer timer;
    bool keep;
    // destroy entity on complete
    bool destroy_entity;
} tween_t;

#ifdef __cplusplus
extern "C" {
#endif

void tween_update(void);
tween_t* tween_reset(entity_t e);

#ifdef __cplusplus
};
#endif

#endif // SCENEX_BASE_TWEEN_H