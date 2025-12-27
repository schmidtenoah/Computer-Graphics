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
#include "instanced.h"

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
static CGMesh *g_models[MODEL_MESH_COUNT];
static GLuint g_textures[TEXTURE_COUNT];

// Texture Idx used for the 6 sides of a cube
static int g_cubeOrder1[] = {0, 0, 0, 1, 0, 0};
static int g_cubeOrder2[] = {2, 2, 2, 1, 2, 2};
 
/**
 * Creates sphere mesh
 */
static void model_initSphere(void) {
    int numSlices = SPHERE_NUM_SLICES;
    int numStacks = SPHERE_NUM_STACKS;

    GLuint numVertices = (numSlices + 1) * (numStacks + 1);
    GLuint numIndices = numSlices * numStacks * 6;

    Vertex* vertices = malloc(sizeof(Vertex) * numVertices);
    GLuint* indices = malloc(sizeof(GLuint) * numIndices);

    int indexVA = 0;
    int indexIA = 0;

    for (int stack = 0; stack <= numStacks; stack++) {
        float stackAngle = ((float)M_PI) / 2 - stack * ((float)M_PI) / numStacks;
        float xy = cosf(stackAngle);
        float z = sinf(stackAngle);

        for (int slice = 0; slice <= numSlices; slice++) {
            float sliceAngle = ((float)M_PI) * slice * 2 / numSlices;
            float x = xy * cosf(sliceAngle);
            float y = xy * sinf(sliceAngle);

            vec3 normal;
            glm_vec3_normalize_to((vec3) { x, y, z }, normal);

            float s = ((float)slice) / numSlices;
            float t = ((float)stack) / numStacks;

            vertices[indexVA++] = (Vertex) {
                x, y, z,                         // Position
                normal[0], normal[1], normal[2], // Normal
                s, t                             // Texture Coordinates
            };

            if (stack < numStacks && slice < numSlices) {
                indices[indexIA++] = stack * (numSlices + 1) + slice;
                indices[indexIA++] = (stack + 1) * (numSlices + 1) + slice;
                indices[indexIA++] = stack * (numSlices + 1) + (slice + 1);

                indices[indexIA++] = stack * (numSlices + 1) + (slice + 1);
                indices[indexIA++] = (stack + 1) * (numSlices + 1) + slice;
                indices[indexIA++] = (stack + 1) * (numSlices + 1) + (slice + 1);
            }
        }
    }

    g_models[MODEL_SPHERE] = instanced_createMesh(vertices, numVertices, indices, numIndices, GL_TRIANGLES);
    free(vertices);
    free(indices);
}

/**
 * Creates Triangle mesh
 */
static void model_initTriangle(void) {
    Vertex triangleVertices[3];
    triangleVertices[0] =  Vertex3Tex(0.0f, 1, 0.0f, 0, 0, 1, 0.5f, 0.5f);
    triangleVertices[1] =  Vertex3Tex(-0.5f, -0.5f, 0.0f, 0, 0, 1, 0.5f, 0.5f);
    triangleVertices[2] =  Vertex3Tex(0.5f, -0.5f, 0.0f, 0, 0, 1, 0.5f, 0.5f);

    g_models[MODEL_TRIANGLE] = instanced_createMesh(triangleVertices, 3, NULL, 0, GL_TRIANGLES);
}

/**
 * Creates Line mesh
 */
static void model_initLine(void) {
    Vertex lineVertices[2];
    lineVertices[0] = Vertex3Tex(-0.5f, 0.0f, 0.0f, 0, 0, 1, 0.0f, 0.0f);
    lineVertices[1] = Vertex3Tex( 0.5f, 0.0f, 0.0f, 0, 0, 1, 1.0f, 1.0f);
    
    g_models[MODEL_LINE] = instanced_createMesh(lineVertices, 2, NULL, 0, GL_LINES);
}

/**
 * Creates cube mesh
 */
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

    g_models[MODEL_CUBE] = instanced_createMesh(vertices, 24, indices, 36, GL_TRIANGLES);
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
    model_initTriangle();
    model_initLine();
    model_loadTextures();

    instanced_init();
    instanced_bindAttrib(g_models[MODEL_SPHERE]);
    instanced_bindAttrib(g_models[MODEL_LINE]);
    instanced_bindAttrib(g_models[MODEL_TRIANGLE]);
}

void model_cleanup(void) {
    for (int i = 0; i < MODEL_MESH_COUNT; ++i) {
        if (g_models[i] != NULL) {
            instanced_disposeMesh(g_models[i]);
            g_models[i] = NULL;
        }
    }

    for (int i = 0; i < TEXTURE_COUNT; ++i) {
        glDeleteTextures(1, &g_textures[i]);
        g_textures[i] = 0;
    }

    instanced_cleanup();
}

void model_drawTextured(ModelType model, bool texOrder1) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    model_bindCubeTextures(texOrder1);
    instanced_draw(g_models[model], false);
}

void model_drawSimple(ModelType model) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setSimpleMVP(false);
    instanced_draw(g_models[model], false);
}

void model_drawInstanced(ModelType model, bool setMVP) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    if (setMVP) {
        shader_setSimpleMVP(true);
    }
    instanced_draw(g_models[model], true);
}
