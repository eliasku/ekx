cmake_minimum_required(VERSION 3.19)
project(sg-file)

add_library(${PROJECT_NAME} STATIC
        src/ek/format/sg_file.c
        )
target_include_directories(${PROJECT_NAME} PUBLIC ./src)

target_link_libraries(${PROJECT_NAME}
        PUBLIC ekx::std
        PUBLIC calo
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})