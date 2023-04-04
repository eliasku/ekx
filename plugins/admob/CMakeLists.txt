cmake_minimum_required(VERSION 3.19)
project(plugin-admob)

add_library(${PROJECT_NAME} STATIC
        src/ek_admob.c
        src/admob_wrapper.c
        )
add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
        PUBLIC plugin-firebase
        PUBLIC ekx::audio
        )

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

    set_source_files_properties("src/ek_admob.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c")

    target_compile_options(${PROJECT_NAME}
            PUBLIC -fno-exceptions
            PUBLIC -fno-rtti
            )
endif()