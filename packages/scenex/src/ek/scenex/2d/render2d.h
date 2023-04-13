#ifndef SCENEX_RENDER2D_H
#define SCENEX_RENDER2D_H

#include <ecx/ecx.h>
#include "transform2d.h"

#ifdef __cplusplus
extern "C" {
#endif

void render2d_draw(entity_t e, const world_transform2d_t* world_transform);

void render2d_draw_stack(entity_t e);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_RENDER2D_H
