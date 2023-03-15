#include "dto.h"
#include <ek/buf.h>

model3d_t create_cube(const vec3_t position, const vec3_t size, color_t color) {
    model3d_t result = {0};

    // 6 sides
    // 6 indices per size
    // 2 faces per size
    uint32_t vertices_count= 6 * 4;
    uint32_t indices_count= 6 * 2 * 3;
    arr_reinit(result.vertices, vertices_count);
    arr_reinit(result.indices, indices_count);

    vec3_t n;
    const float u = 0.5f;
    const color_t color1 = color;
    const color_t color2 = COLOR_ZERO;

    // Top +Z
    n = vec3(0, 0, 1);
    result.vertices[0] = (model3d_vertex_t){vec3(-u, -u, u), n, vec2(0, 0), color1, color2};
    result.vertices[1] = (model3d_vertex_t){vec3(u, -u, u), n, vec2(0, 1), color1, color2};
    result.vertices[2] = (model3d_vertex_t){vec3(u, u, u), n, vec2(1, 1), color1, color2};
    result.vertices[3] = (model3d_vertex_t){vec3(-u, u, u), n, vec2(1, 0), color1, color2};

    n = vec3(0, 0, -1);
    result.vertices[4] = (model3d_vertex_t){vec3(-u, -u, -u), n, vec2(0, 0), color1, color2};
    result.vertices[5] = (model3d_vertex_t){vec3(-u, u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[6] = (model3d_vertex_t){vec3(u, u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[7] = (model3d_vertex_t){vec3(u, -u, -u), n, vec2(1, 0), color1, color2};

    n = vec3(1, 0, 0);
    result.vertices[8] = (model3d_vertex_t){vec3(u, -u, u), n, vec2(0, 0), color1, color2};
    result.vertices[9] = (model3d_vertex_t){vec3(u, -u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[10] =(model3d_vertex_t) {vec3(u, u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[11] =(model3d_vertex_t) {vec3(u, u, u), n, vec2(1, 0), color1, color2};

    n = vec3(-1, 0, 0);
    result.vertices[12] = (model3d_vertex_t){vec3(-u, u, u), n, vec2(0, 0), color1, color2};
    result.vertices[13] = (model3d_vertex_t){vec3(-u, u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[14] = (model3d_vertex_t){vec3(-u, -u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[15] = (model3d_vertex_t){vec3(-u, -u, u), n, vec2(1, 0), color1, color2};

    n = vec3(0, -1, 0);
    result.vertices[16] = (model3d_vertex_t){vec3(-u, -u, u), n, vec2(0, 0), color1, color2};
    result.vertices[17] = (model3d_vertex_t){vec3(-u, -u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[18] = (model3d_vertex_t){vec3(u, -u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[19] = (model3d_vertex_t){vec3(u, -u, u), n, vec2(1, 0), color1, color2};

    n = vec3(0, 1, 0);
    result.vertices[20] = (model3d_vertex_t){vec3(u, u, u), n, vec2(0, 0), color1, color2};
    result.vertices[21] = (model3d_vertex_t){vec3(u, u, -u), n, vec2(0, 1), color1, color2};
    result.vertices[22] = (model3d_vertex_t){vec3(-u, u, -u), n, vec2(1, 1), color1, color2};
    result.vertices[23] = (model3d_vertex_t){vec3(-u, u, u), n, vec2(1, 0), color1, color2};

    // 12 * 3 = 36
    const uint16_t indices[36] = {
            0, 1, 2,
            2, 3, 0,
            4, 5, 6,
            6, 7, 4,
            8, 9, 10,
            10, 11, 8,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };
    memcpy(result.indices, indices, sizeof(uint16_t) * 36);

    for (uint32_t i = 0; i < vertices_count; ++i) {
        model3d_vertex_t* v = result.vertices + i;
        v->position.x = position.x + v->position.x * size.x;
        v->position.y = position.y + v->position.y * size.y;
        v->position.z = position.z + v->position.z * size.z;
    }

    return result;
}

model3d_t create_plane(const vec3_t position, const vec2_t size, color_t color) {
    model3d_t result = {0};

    const float u = 0.5f;
    const color_t color1 = color;
    const color_t color2 = COLOR_ZERO;
    const vec3_t n = vec3(0, 0, 1);

    uint32_t vertices_count= 4;
    uint32_t indices_count= 2 * 3;
    arr_reinit(result.vertices, vertices_count);
    arr_reinit(result.indices, indices_count);

    result.vertices[0] = (model3d_vertex_t){vec3(-u, -u, 0), n, vec2(0, 0), color1, color2};
    result.vertices[1] = (model3d_vertex_t){vec3(u, -u, 0), n, vec2(1, 0), color1, color2};
    result.vertices[2] = (model3d_vertex_t){vec3(u, u, 0), n, vec2(1, 1), color1, color2};
    result.vertices[3] = (model3d_vertex_t){vec3(-u, u, 0), n, vec2(0, 1), color1, color2};

    result.indices[0] = 0;
    result.indices[1] = 1;
    result.indices[2] = 2;
    result.indices[3] = 2;
    result.indices[4] = 3;
    result.indices[5] = 0;

    for (uint32_t i = 0; i < vertices_count; ++i) {
        model3d_vertex_t* v = result.vertices + i;
        v->position.x = position.x + v->position.x * size.x;
        v->position.y = position.y + v->position.y * size.y;
        v->position.z = position.z + v->position.z;
    }

    return result;
}
