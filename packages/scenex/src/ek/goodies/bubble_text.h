#ifndef SCENE_GOODIES_BUBBLE_TEXT_H
#define SCENE_GOODIES_BUBBLE_TEXT_H

#include <ek/math.h>
#include <ek/hash.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float delay;
    float time;
    vec2_t start;
    vec2_t offset;
} bubble_text_t;

entity_t bubble_text_entity(string_hash_t font_name, vec2_t pos, float delay);
void bubble_text_update(void);
void bubble_text_setup(void);

#ifdef __cplusplus
}
#endif

#endif // SCENE_GOODIES_BUBBLE_TEXT_H
