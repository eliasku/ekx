#ifndef SCENEX_HIT_TEST_H
#define SCENEX_HIT_TEST_H

#include <ecx/ecx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

entity_t hit_test_2d(entity_t e, vec2_t parentSpacePosition);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_HIT_TEST_H
