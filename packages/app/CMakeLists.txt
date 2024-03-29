cmake_minimum_required(VERSION 3.19)

project(app C)

add_library(${PROJECT_NAME} STATIC
        src/app/app.c
        src/local_storage/local_storage.c
        src/local_res/local_res.c
        src/texture_loader/texture_loader.c
        )

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_options(${PROJECT_NAME}
        PRIVATE -g
        #PRIVATE -flto
        PRIVATE -Os
        PRIVATE -Wall
        PRIVATE -Wextra
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_options(${PROJECT_NAME}
        PRIVATE -g
        #PRIVATE -flto
        PRIVATE -Os
        PRIVATE -fno-strict-aliasing
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ekx::std)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_options(${PROJECT_NAME}
            PUBLIC "SHELL:--js-library ${CMAKE_CURRENT_SOURCE_DIR}/js/lib/app.js"
            PUBLIC "SHELL:--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/js/pre/app.js"
            )

    target_link_libraries(${PROJECT_NAME}
            PUBLIC GLESv2
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework CoreText"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework UIKit"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            )
    target_sources(${PROJECT_NAME} PRIVATE
            src/apple/app_apple.m
            src/local_storage/local_storage.m
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${PROJECT_NAME}
            PRIVATE "-framework CoreText"
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE "-framework QuartzCore"
            )
    target_sources(${PROJECT_NAME} PRIVATE
            src/app/app_apple.m
            src/local_storage/local_storage.m
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME}
            PUBLIC GL dl m)
else ()
    error("Platform: UNKNOWN")
endif ()
