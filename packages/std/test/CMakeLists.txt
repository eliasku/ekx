cmake_minimum_required(VERSION 3.19)
project(std-tests C)

add_executable(${PROJECT_NAME}
        std_test_main.c
        bitmap/bitmap_test.c
        math/math_easing_test.c
        math/math_mat_test.c
        math/math_rect_test.c
        math/math_test_common.h
        math/math_vec_test.c
        base64_test.c
        log_test.c
        path_test.c
        rnd_test.c
        string_test.c
        hash_test.c
        sigslot_test.c
        legacy_ds/Array_test.cpp
        legacy_ds/Hash_test.cpp
        )

add_test(NAME ${PROJECT_NAME} COMMAND ${NODE_JS_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>)
test_code_coverage(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11 CXX_STANDARD 17)
target_compile_definitions(${PROJECT_NAME} PUBLIC UNIT_TESTING)

target_compile_options(${PROJECT_NAME}
        PUBLIC -g
        #PUBLIC -flto
        PUBLIC -O0
        PUBLIC -fno-strict-aliasing
        PUBLIC -ffast-math
        PUBLIC -Wstrict-aliasing=2
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ekx::std)
