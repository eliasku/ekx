cmake_minimum_required(VERSION 3.19)
project(graphics)

add_library(${PROJECT_NAME} STATIC
        src/gfx.c
        src/canvas.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::app
        PUBLIC ekx::headers
        PUBLIC ekx::std
        )

if (EKX_BUILD_DEV_TOOLS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SOKOL_TRACE_HOOKS)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_compile_options(${PROJECT_NAME} PRIVATE
        -g
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin|iOS")

    set_source_files_properties("src/gfx.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

else ()

endif ()
