#ifndef SCENE_GOODIES_BUBBLE_TEXT_H
#define SCENE_GOODIES_BUBBLE_TEXT_H

#include <ek/math.h>
#include <ek/hash.h>
#include <ecx/ecx.h>

typedef struct {
    float delay;
    float time;
    vec2_t start;
    vec2_t offset;
} bubble_text_t;

#ifdef __cplusplus
extern "C" {
#endif

entity_t bubble_text_entity(string_hash_t fontName, vec2_t pos, float delay);
void bubble_text_update(void);

#ifdef __cplusplus
}
#endif

#endif // SCENE_GOODIES_BUBBLE_TEXT_H
