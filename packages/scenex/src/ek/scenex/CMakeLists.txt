cmake_minimum_required(VERSION 3.19)

target_sources(${PROJECT_NAME} PUBLIC

        3d/render_system.c.h

        ekx_app.cpp
        ekx_app.c

        base/node.h
        base/node.c
        base/interactive.c
        base/node_events.h
        base/NodeEvents.cpp
        base/tween.h
        base/destroy_timer.h

        2d/atlas.c
        2d/sprite.c
        2d/dynamic_atlas.c

        2d/transform2d.c
        2d/display2d.c
        2d/text2d.c
        2d/render2d.c
        2d/render2d.h
        2d/camera2d.h
        2d/camera2d.c
        2d/camera2d_debug.c
        2d/layout_rect.h
        2d/layout_rect.c
        2d/viewport.h
        2d/viewport.c
        2d/button.h
        2d/button.c
        2d/movieclip.h
        2d/movieclip.c

        text/font.c
        text/text_engine.c
        text/text_format.c

        interaction_system.c
        interaction_system.h

        particles/particle.c
        particles/particle.h
        particles/particle_decl.c
        particles/particle_system.c
        particles/particle_system.h

        app/basic_application.cpp
        app/root_app_callbacks.c
        app/game_app_callbacks.c

        systems/hit_test.h
        systems/hit_test.c
        systems/main_flow.h
        systems/main_flow.c

        assets/Assets.cpp
        assets/asset_manager.c

        scene_factory.h
        scene_factory.c
        )