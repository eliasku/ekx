cmake_minimum_required(VERSION 3.19)
project(std C)

add_library(${PROJECT_NAME} STATIC
        src/ek_std.c
        src/ek_bitmap.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_code_coverage(${PROJECT_NAME} PUBLIC)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_options(${PROJECT_NAME}
        PRIVATE -g
        #PRIVATE -flto
        PRIVATE -Os
        PRIVATE -Wall
        PRIVATE -Wextra
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_options(${PROJECT_NAME}
        PRIVATE -g
        #PRIVATE -flto
        PRIVATE -Os
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ekx::headers)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|iOS")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(${PROJECT_NAME}
            PUBLIC log
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC m Threads::Threads)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME}
            PUBLIC m
            )
else ()
    error("Platform: UNKNOWN")
endif ()

if (EKX_BUILD_TESTS)
    add_subdirectory(test)
endif ()
