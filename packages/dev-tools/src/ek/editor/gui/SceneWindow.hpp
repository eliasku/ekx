#pragma once

#include <ekx/app/game_display.h>

struct SceneView2D {
    float scaleMin = 0.1f;
    float scale = 1.0f;
    vec2_t position = {};
    mat3x2_t matrix = mat3x2_identity();
    vec2_t translation = {};
    mat4_t viewMatrix3D = mat4_identity();
    mat4_t projectionMatrix= mat4_identity();

    [[nodiscard]]
    vec2_t getMouseWorldPos(vec2_t viewportMousePosition) const;
    void reset();
    void manipulateView(vec2_t mouseWorldPosition, rect_t viewport);
};

struct SceneView3D {
    vec3_t position = {};
    vec3_t translation = {};
    mat4_t viewMatrix = mat4_identity();
    mat4_t projectionMatrix= mat4_identity();

    [[nodiscard]]
    vec2_t getMouseWorldPos(vec2_t viewportMousePosition) const;
    void reset();
//    void manipulateView();
};

struct SceneView {
    SceneView2D view2;
    SceneView3D view3;
    bool mode2D = true;
    rect_t rect = rect_01();

    [[nodiscard]]
    vec2_t getMouseWorldPos() const;
    void reset();
    void manipulateView();
};


struct SceneWindow {
    SceneWindow();

    ~SceneWindow();

    game_display display = {};
    SceneView view{};

    entity_t root = NULL_ENTITY;
    entity_t hoverTarget = NULL_ENTITY;

    int currentTool = 0;
    int localGlobal = 0;

    void pre_render();

    void drawScene();

    static void drawSceneNode(entity_t e);
    void drawSceneNodeBounds(entity_t e);

    entity_t hitTest(entity_t e, vec2_t worldPos);

    void drawToolbar();

    void manipulateObject2D();
    void manipulateObject3D();
};

extern SceneWindow editor_scene_window;
void draw_scene_window(void);
