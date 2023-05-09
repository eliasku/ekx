cmake_minimum_required(VERSION 3.19)

add_library(${PROJECT_NAME} STATIC

        ek/editor/gui/EditorGUI.cpp
        ek/editor/gui/Widgets_impl.hpp

        ek/editor/imgui/cimgui.cpp
        ek/editor/imgui/ekimgui.h
        ek/editor/imgui/ekimgui.cpp
        ek/editor/imgui/imgui.hpp

        ek/editor/Editor.cpp
        ek/editor/Editor.hpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 11
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_compile_definitions(${PROJECT_NAME} PUBLIC
        EK_DEV_TOOLS
        EK_DEV_TOOLS_DEBUG
        )

if(NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
target_compile_options(${PROJECT_NAME} PUBLIC
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        #        -fno-exceptions
        )
endif()

target_compile_options(${PROJECT_NAME}
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -fno-strict-aliasing
        PRIVATE -Wstrict-aliasing=2
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC scenex
        PUBLIC ekx::std
        PUBLIC imgui
        PUBLIC calo
        )
