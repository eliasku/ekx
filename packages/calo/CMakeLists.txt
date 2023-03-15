cmake_minimum_required(VERSION 3.19)
project(calo C)

add_library(${PROJECT_NAME} STATIC src/calo-all.c)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)
target_link_libraries(${PROJECT_NAME} PUBLIC ekx::std)

add_executable(${PROJECT_NAME}_test test.c)
target_link_libraries(${PROJECT_NAME}_test PUBLIC ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}_test PROPERTIES C_STANDARD 11)

