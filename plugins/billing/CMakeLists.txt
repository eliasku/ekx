cmake_minimum_required(VERSION 3.19)
project(plugin-billing)
add_library(${PROJECT_NAME} STATIC
        src/billing.hpp
        src/billing.cpp
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::scenex)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

    set_source_files_properties("src/billing.cpp" PROPERTIES
            COMPILE_FLAGS "-x objective-c++")

endif()

target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-exceptions
        -fno-rtti
        -fno-strict-aliasing
        -Wstrict-aliasing=2
        )
