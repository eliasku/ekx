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
    TimeLayer timer;
    bool keep;
    // destroy entity on complete
    bool destroy_entity;
} tween_t;

void tween_update(void);
tween_t* tween_reset(entity_t e);

extern ecx_component_type Tween;
void Tween_setup(void);
#define get_tween(e) ((tween_t*)get_component(&Tween, e))
#define add_tween(e) ((tween_t*)add_component(&Tween, e))
#define del_tween(e) (remove_component(&Tween, e))

#ifdef __cplusplus
};
#endif

#endif // SCENEX_TWEEN_H
