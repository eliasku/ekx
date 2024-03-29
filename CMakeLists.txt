cmake_minimum_required(VERSION 3.19)
project(ekx)

option(EKX_BUILD_DEV_TOOLS "Build dev tools" OFF)
option(EKX_BUILD_TESTS "Build tests" OFF)
option(EKX_BUILD_EXTERNAL_TESTS "Build external modules tests" OFF)
option(EKX_BUILD_COVERAGE "Collect coverage info during test run" OFF)
option(EKX_INCLUDE_PLUGINS "Include plugin projects" ON)

if (EKX_BUILD_TESTS OR EKX_BUILD_EXTERNAL_TESTS)
    enable_testing()
    if (EKX_BUILD_COVERAGE)
        set(CODE_COVERAGE ON)
    endif ()
endif ()

include(packages/std/cmake/code-coverage.cmake)

add_library(${PROJECT_NAME} INTERFACE)

add_subdirectory(external)

add_subdirectory(packages/std)
add_subdirectory(packages/calo)
add_subdirectory(packages/app)
add_subdirectory(packages/ecs)
add_subdirectory(packages/graphics)
add_subdirectory(packages/audio)
add_subdirectory(packages/sce)
add_subdirectory(packages/scenex)
add_subdirectory(packages/appbox)

if (EKX_BUILD_DEV_TOOLS)
    add_subdirectory(packages/dev-tools)
endif ()

if (EKX_INCLUDE_PLUGINS)
    add_subdirectory(plugins/billing)
    add_subdirectory(plugins/game-services)
    add_subdirectory(plugins/firebase)
    add_subdirectory(plugins/admob)
endif ()

if ((NOT EMSCRIPTEN) AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "iOS")) AND (NOT (${CMAKE_SYSTEM_NAME} MATCHES "Android")))
    add_subdirectory(tools/ekc)
    add_subdirectory(tools/flash-export)
endif ()

if (EKX_BUILD_COVERAGE)
    coverage_all()
endif ()