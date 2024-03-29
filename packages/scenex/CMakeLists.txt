cmake_minimum_required(VERSION 3.19)
project(scenex)

add_library(${PROJECT_NAME} STATIC)

add_library(ekx::scenex ALIAS scenex)

option(EK_UITEST "Enable features for UI testing" OFF)

if(EK_UITEST)
    message(STATUS "UI Testing enabled")
    add_definitions(-DEK_UITEST)
endif(EK_UITEST)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

add_subdirectory(src/ek/scenex)
add_subdirectory(src/ek/goodies)

target_include_directories(${PROJECT_NAME}
        PUBLIC src
        PUBLIC include
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::graphics
        PUBLIC ekx::audio
        PUBLIC ekx::app
        PUBLIC ekx::ecs
        PUBLIC calo
        PUBLIC ekx::sce
        )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-dollar-in-identifier-extension
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
endif()