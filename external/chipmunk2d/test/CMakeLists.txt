cmake_minimum_required(VERSION 3.19)
project(chipmunk2d-test)
add_executable(${PROJECT_NAME} main.c)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_link_libraries(${PROJECT_NAME} PUBLIC chipmunk2d)
add_test(NAME ${PROJECT_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${PROJECT_NAME})