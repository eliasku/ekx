cmake_minimum_required(VERSION 3.19)

target_sources(${PROJECT_NAME} PRIVATE
        GameScreen.cpp

        popup_manager.c
        camera_shaker.c
        bubble_text.c

        simple_animator.c
        fireworks.cpp

        helpers/Trail2D.cpp
        )