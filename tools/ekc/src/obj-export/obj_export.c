#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj/fast_obj.h>

#include <ek/log.h>
#include <ek/math.h>
#include <gen_sg.h>

typedef model3d_vertex_t vertex3d_t;

int convertObjModel(const char* input, const char* output) {
    fastObjMesh* mesh = fast_obj_read(input);
    if (!mesh) {
        return 1;
    }

    unsigned int verticesCount = 0;
    unsigned int indicesCount = 0;
    for (uint32_t i = 0; i < mesh->face_count; ++i) {
        unsigned int faceVerticesCount = mesh->face_vertices[i];
        verticesCount += faceVerticesCount;
        indicesCount += faceVerticesCount;
    }

    if (indicesCount > 0 && verticesCount > 0) {
        vertex3d_t* vertices = (vertex3d_t*) malloc(sizeof(vertex3d_t) * verticesCount);
        uint16_t* indices = (uint16_t*) malloc(sizeof(uint16_t) * indicesCount);
        vertex3d_t* pVertex = vertices;
        uint16_t* pIndex = indices;

        uint32_t vi = 0;
        for (uint32_t i = 0; i < mesh->face_count; ++i) {
            const uint32_t faceVerticesCount = mesh->face_vertices[i];
            for (uint32_t j = 0; j < faceVerticesCount; ++j) {
                const fastObjIndex face = mesh->indices[vi];
                const float* p = mesh->positions + face.p * 3;
                const float* n = mesh->normals + face.n * 3;
                const float* t = mesh->texcoords + face.t * 2;

                pVertex->position.x = p[0];
                pVertex->position.y = p[1];
                pVertex->position.z = p[2];
                pVertex->normal.x = n[0];
                pVertex->normal.y = n[1];
                pVertex->normal.z = n[2];
                pVertex->uv.x = t[0];
                pVertex->uv.y = t[1];
                pVertex->color = COLOR_WHITE;
                pVertex->color2 = COLOR_ZERO;
                ++pVertex;

                *pIndex = vi;
                ++pIndex;

                ++vi;
            }
        }

        fast_obj_destroy(mesh);

        calo_writer_t writer = new_writer(100);
        write_u32(&writer, verticesCount);
        write_span(&writer, vertices, sizeof(vertex3d_t) * verticesCount);
        write_u32(&writer, indicesCount);
        write_span(&writer, indices, sizeof(uint16_t) * indicesCount);

        FILE* f = fopen(output, "wb");
        fwrite_calo(f, &writer);
        fclose(f);

        free(vertices);
        free(indices);
        return 0;
    }

    fast_obj_destroy(mesh);
    return 1;
}
