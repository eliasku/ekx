#include "math_test_common.h"

SUITE(rect) {

    IT("is empty") {
        CHECK(rect_is_empty(rect(0, 0, 0, 0)));
        CHECK(rect_is_empty(rect(1, 1, -1, -1)));
        CHECK(rect_is_empty(rect(0, 0, 0, 1)));
        CHECK(rect_is_empty(rect(0, 0, 1, 0)));
        CHECK_FALSE(rect_is_empty(rect(0, 0, 1, 1)));

        CHECK(aabb2_is_empty((aabb2_t){0, 0, 0, 0}));
        CHECK(aabb2_is_empty(aabb2_empty()));
        CHECK(aabb2_is_empty((aabb2_t){1, 1, 1, 1}));
        CHECK(aabb2_is_empty((aabb2_t){1, -1, 1, 1}));
        CHECK(aabb2_is_empty((aabb2_t){-1, 1, 1, 1}));
        CHECK(aabb2_is_empty((aabb2_t){-1, -1, -1, -1}));
        CHECK_FALSE(aabb2_is_empty((aabb2_t){-1, -1, 1, 1}));
    }

}
