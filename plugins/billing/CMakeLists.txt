cmake_minimum_required(VERSION 3.19)
project(plugin-billing)
add_library(${PROJECT_NAME} STATIC
        src/billing.h
        src/billing.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::scenex)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

    set_source_files_properties("src/billing.c" PROPERTIES
            COMPILE_FLAGS "-x objective-c++")

endif()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )
