#ifndef SCENEX_MAIN_FLOW_H
#define SCENEX_MAIN_FLOW_H

#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

void scene_pre_update(entity_t root, float dt);
void scene_post_update(entity_t root);
void scene_render(entity_t root);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_MAIN_FLOW_H
