cmake_minimum_required(VERSION 3.19)
project(dev-tools)

add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

## THAT's how you enable dev-tools in your project
if (EKX_BUILD_DEV_TOOLS)
    add_compile_definitions(EK_DEV_TOOLS)
    target_link_libraries(scenex
            PUBLIC ${PROJECT_NAME}
            )
endif ()