/**
 * @file model.c
 * @brief Implementation of model creation and management
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include <fhwcg/fhwcg.h>
#include "model.h"
#include "shader.h"
#include "rendering.h"

#define SPHERE_NUM_SLICES 20
#define SPHERE_NUM_STACKS 20

#define TEXTURE_COUNT 7

#define TEXTURE_PATH RESOURCE_PATH "textures/"
#define TEX0 TEXTURE_PATH "tile.png"
#define TEX1 TEXTURE_PATH "arcade_carpet_1_512.png"

#define SKYBOX_PATH TEXTURE_PATH "gloomy_skybox/"
#define TEX2 SKYBOX_PATH "gloomy_up.png" 
#define TEX3 SKYBOX_PATH "gloomy_dn.png" 
#define TEX4 SKYBOX_PATH "gloomy_rt.png" 
#define TEX5 SKYBOX_PATH "gloomy_lf.png" 
#define TEX6 SKYBOX_PATH "gloomy_ft.png"

////////////////////////    LOCAL    ////////////////////////////

/** Array of mesh models */
static Mesh *g_models[MODEL_MESH_COUNT];
static GLuint g_textures[TEXTURE_COUNT];

// Texture Idx used for the 6 sides of a cube
static int g_cubeOrder1[] = {0, 0, 0, 1, 0, 0};
static int g_cubeOrder2[] = {2, 2, 2, 1, 2, 2};
 
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

/**
 * Binds the face textures for the texture shader.
 * @param useOrder1 Which texture order is wanted.
 */
static void model_bindCubeTextures(bool useOrder1) {
    int *order = useOrder1 ? g_cubeOrder1 : g_cubeOrder2;
    int units[6] = {0, 1, 2, 3, 4, 5};
    Shader *shader = shader_getTextureShader();
    shader_useShader(shader);
    for (int i = 0; i < 6; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, g_textures[order[i]]);
    }

    shader_setIntN(shader, "u_textures", units, 6);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(shader, "u_mvpMatrix", &mat);
}

/**
 * Loads textures for room
 */
static void model_loadTextures(void) {
    g_textures[0] = texture_loadTexture(TEX0, GL_REPEAT);
    g_textures[1] = texture_loadTexture(TEX1, GL_REPEAT);
    g_textures[2] = texture_loadTexture(TEX2, GL_REPEAT);
    g_textures[3] = texture_loadTexture(TEX3, GL_REPEAT);
    g_textures[4] = texture_loadTexture(TEX4, GL_REPEAT);
    g_textures[5] = texture_loadTexture(TEX5, GL_REPEAT);
    g_textures[6] = texture_loadTexture(TEX6, GL_REPEAT);
}

////////////////////////    PUBLIC    ////////////////////////////

void model_init(void) {
    model_initSphere();
    model_initCube();
    model_loadTextures();
}

void model_cleanup(void) {
    for (int i = 0; i < MODEL_MESH_COUNT; ++i) {
        if (g_models[i] != NULL) {
            mesh_disposeMesh(&g_models[i]);
            g_models[i] = NULL;
        }
    }

    for (int i = 0; i < TEXTURE_COUNT; ++i) {
        glDeleteTextures(1, &g_textures[i]);
        g_textures[i] = 0;
    }
}

void model_drawTextured(ModelType model, bool texOrder1) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    model_bindCubeTextures(texOrder1);
    mesh_drawMesh(g_models[model]);
}

void model_drawSimple(ModelType model) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setSimpleMVP();
    mesh_drawMesh(g_models[model]);
}