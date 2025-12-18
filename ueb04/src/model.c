/**
 * @file model.c
 * @brief Implementation of model creation and management
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "model.h"
#include "shader.h"
#include "rendering.h"

#define SPHERE_NUM_SLICES 20
#define SPHERE_NUM_STACKS 20

////////////////////////    LOCAL    ////////////////////////////

/** Array of mesh models */
static Mesh *g_models[MODEL_MESH_COUNT];

/**
 * Creates sphere mesh
 */
static void model_initSphere(void) {
    g_models[MODEL_SPHERE] = mesh_createSphere(SPHERE_NUM_SLICES, SPHERE_NUM_STACKS);
}

/**
 * Creates cube mesh
 */
// TODO: Gibts hier nicht was schnelles?
static void model_initCube(void) {
    // 6 faces * 4 vertices per face = 24 vertices
    const float positions[24][3] = {
        // +X face
        { 1, -1, -1 }, { 1,  1, -1 }, { 1,  1,  1 }, { 1, -1,  1 },
        // -X face
        {-1, -1,  1 }, {-1,  1,  1 }, {-1,  1, -1 }, {-1, -1, -1 },
        // +Y face
        {-1,  1, -1 }, {-1,  1,  1 }, { 1,  1,  1 }, { 1,  1, -1 },
        // -Y face
        {-1, -1,  1 }, {-1, -1, -1 }, { 1, -1, -1 }, { 1, -1,  1 },
        // +Z face
        {-1, -1,  1 }, { 1, -1,  1 }, { 1,  1,  1 }, {-1,  1,  1 },
        // -Z face
        { 1, -1, -1 }, {-1, -1, -1 }, {-1,  1, -1 }, { 1,  1, -1 }
    };

    const float normals[6][3] = {
        { 1,  0,  0 }, {-1,  0,  0 }, // +X, -X
        { 0,  1,  0 }, { 0, -1,  0 }, // +Y, -Y
        { 0,  0,  1 }, { 0,  0, -1 }  // +Z, -Z
    };

    const float texCoords[4][2] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
    };

    Vertex vertices[24];
    GLuint indices[36];

    int vIndex = 0;
    int iIndex = 0;

    for (int face = 0; face < 6; face++) {
        for (int vert = 0; vert < 4; vert++) {
            vertices[vIndex++] = (Vertex){
                positions[face * 4 + vert][0],
                positions[face * 4 + vert][1],
                positions[face * 4 + vert][2],
                normals[face][0], normals[face][1], normals[face][2],
                texCoords[vert][0], texCoords[vert][1]
            };
        }

        int base = face * 4;
        indices[iIndex++] = base + 0;
        indices[iIndex++] = base + 1;
        indices[iIndex++] = base + 2;
        indices[iIndex++] = base + 0;
        indices[iIndex++] = base + 2;
        indices[iIndex++] = base + 3;
    }

    g_models[MODEL_CUBE] = mesh_createMesh("Cube", vertices, 24, indices, 36, GL_TRIANGLES);
}

////////////////////////    PUBLIC    ////////////////////////////

void model_init(void) {
    model_initSphere();
    model_initCube();
}

void model_loadTextures(void) {
    // No textures yet
}

GLuint model_getTextureId(int index) {
    (void)index;
    return 0;
}

void model_cleanup(void) {
    for (int i = 0; i < MODEL_MESH_COUNT; ++i) {
        if (g_models[i] != NULL) {
            mesh_disposeMesh(&g_models[i]);
            g_models[i] = NULL;
        }
    }
}

void model_drawTextured(ModelType model) {
    model_drawSimple(model);
}

void model_drawSimple(ModelType model) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setSimpleMVP();
    mesh_drawMesh(g_models[model]);
}