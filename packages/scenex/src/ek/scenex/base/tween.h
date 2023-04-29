#ifndef SCENEX_TWEEN_H
#define SCENEX_TWEEN_H

#include <ecx/ecx.h>
#include <ekx/app/time_layers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (* advanced)(entity_t e, float t);

    void (* completed)(entity_t e);

    int data[4];
    float delay;
    float time;
    float duration;
    time_layer_t timer;
    bool keep;
    // destroy entity on complete
    bool destroy_entity;
} tween_t;

void update_tweens(void);
tween_t* tween_reset(entity_t e);

extern ECX_DEFINE_TYPE(tween_t);
void setup_tween(void);
#define get_tween(e) ECX_GET(tween_t,e)
#define add_tween(e) ECX_ADD(tween_t,e)
#define del_tween(e) ECX_DEL(tween_t,e)

#ifdef __cplusplus
};
#endif

#endif // SCENEX_TWEEN_H
