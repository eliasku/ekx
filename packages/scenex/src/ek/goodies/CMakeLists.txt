cmake_minimum_required(VERSION 3.19)

target_sources(${PROJECT_NAME} PRIVATE
        GameScreen.cpp

        PopupManager.cpp
        camera_shaker.c
        bubble_text.cpp

        simple_animator.c
        fireworks.cpp

        helpers/follow.cpp
        helpers/Trail2D.cpp
        )