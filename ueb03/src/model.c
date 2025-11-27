/**
 * @file model.c
 * @brief Implementation of model creation and rendering.
 *
 * Creates unit models (circle, square, star, triangle) as static
 * meshes and manages a curve with VAO/VBO for real-time updates.
 * Handles vertex setup and buffer updates.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "model.h"
#include "shader.h"
#include "rendering.h"
#include "input.h"

#define SPHERE_NUM_SLICES 12
#define SPHERE_NUM_STACKS SPHERE_NUM_SLICES

#define SURFACE_DEFAULT_SIZE 16
#define NUM_TEXTURES 3

///////////////////////    LOCAL    ////////////////////////////

/** Array of pointers to mesh models (circle, square, star, triangle) */
static Mesh *g_models[MODEL_MESH_COUNT];

/** Texture IDs for surface textures */
static GLuint g_textureIds[NUM_TEXTURES] = {0};

static struct {
    GLuint vao, vbo, ebo;
    size_t vertexBufferSize;
    size_t indexBufferSize;
    int numVertices;
    int numIndices;
} g_surface = {
    .vao = 0, .vbo = 0, .ebo = 0,
    .vertexBufferSize = SURFACE_DEFAULT_SIZE * sizeof(Vertex),
    .indexBufferSize = SURFACE_DEFAULT_SIZE * 6 * sizeof(GLuint),
    .numVertices = 0,
    .numIndices = 0
};

/**
 * Creates a unit Sphere mesh.
 * Center: (0,0)
 * Radius: 1.
 */
static void model_initSphere(void) {
    g_models[MODEL_SPHERE] = mesh_createSphere(SPHERE_NUM_SLICES, SPHERE_NUM_STACKS);
}

/**
 * Initializes the vao, vbo and ebo for the surface mesh.
 * VBO and EBO are supposed to change dynamically.
 */
static void model_initSurface(void) {
    glGenVertexArrays(1, &g_surface.vao);
    glGenBuffers(1, &g_surface.vbo);
    glGenBuffers(1, &g_surface.ebo);

    glBindVertexArray(g_surface.vao);

    // Vertex Buffer (Dynamic)
    glBindBuffer(GL_ARRAY_BUFFER, g_surface.vbo);
    glBufferData(GL_ARRAY_BUFFER, g_surface.vertexBufferSize, NULL, GL_DYNAMIC_DRAW);

    // Index Buffer (Dynamic)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_surface.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_surface.indexBufferSize, NULL, GL_DYNAMIC_DRAW);

    // Vertex Attribute Layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

///////////////////////    PUBLIC    ////////////////////////////

void model_init(void) {
    model_initSphere();
    model_initSurface();
    model_loadTextures();
}

void model_loadTextures(void) {
    const char* texturePaths[NUM_TEXTURES] = {
        RESOURCE_PATH "textures/texture1.jpg",
        RESOURCE_PATH "textures/texture2.jpg",
        RESOURCE_PATH "textures/texture3.jpg"
    };
    
    for (int i = 0; i < NUM_TEXTURES; i++) {
        g_textureIds[i] = texture_loadTexture(texturePaths[i], GL_REPEAT);
        
        // Set texture parameters
        glBindTexture(GL_TEXTURE_2D, g_textureIds[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint model_getTextureId(int index) {
    if (index < 0 || index >= NUM_TEXTURES) {
        return g_textureIds[0];
    }
    return g_textureIds[index];
}

void model_cleanup(void) {
    for (int i = 0; i < MODEL_MESH_COUNT; ++i) {
        if (g_models[i] == NULL) {
            continue;
        }

        mesh_disposeMesh(&(g_models[i]));
        g_models[i] = NULL;
    }
    
    // Cleanup textures
    for (int i = 0; i < NUM_TEXTURES; i++) {
        if (g_textureIds[i] != 0) {
            texture_deleteTexture(&g_textureIds[i]);
        }
    }
    
    glDeleteBuffers(1, &g_surface.vbo);
    glDeleteBuffers(1, &g_surface.ebo);
    glDeleteVertexArrays(1, &g_surface.vao);
}

void model_draw(ModelType model, bool drawNormals, bool useBallMat, mat4 *viewMat, mat4 *modelviewMat) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setMVP(viewMat, modelviewMat, useBallMat);
    mesh_drawMesh(g_models[model]);

    if (drawNormals) {
        shader_setNormals();
        mesh_drawMesh(g_models[model]);
    }
}

void model_drawSimple(ModelType model) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setSimpleMVP();
    mesh_drawMesh(g_models[model]);
}

void model_drawSurface(bool drawNormals, mat4 *viewMat, mat4 *modelviewMat) {
    glBindVertexArray(g_surface.vao);

    shader_setMVP(viewMat, modelviewMat, false);
    glDrawElements(GL_TRIANGLES, g_surface.numIndices, GL_UNSIGNED_INT, 0);

    if (drawNormals) {
        shader_setNormals();
        glDrawElements(GL_TRIANGLES, g_surface.numIndices, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void model_updateSurface(vec3 *vertices, vec3 *normals, vec2 *texcoords, int dim) {
    int numVertices = dim * dim;
    int numIndices = (dim - 1) * (dim - 1) * 6;

    Vertex *vdata = malloc(numVertices * sizeof(Vertex));
    GLuint *indices = malloc(numIndices * sizeof(GLuint));

    for (int y = 0; y < dim; y++) {
        for (int x = 0; x < dim; x++) {
            int i = y * dim + x;

            glm_vec3_copy(vertices[i], vdata[i].position);
            glm_vec3_copy(normals[i], vdata[i].normal);
            glm_vec2_copy(texcoords[i], vdata[i].texCoords);
        }
    }

    // Indizes erzeugen
    int idx = 0;
    for (int y = 0; y < dim - 1; y++) {
        for (int x = 0; x < dim - 1; x++) {
            GLuint v0 = y * dim + x;
            GLuint v1 = v0 + 1;
            GLuint v2 = v0 + dim;
            GLuint v3 = v2 + 1;

            indices[idx++] = v0; indices[idx++] = v2; indices[idx++] = v1;
            indices[idx++] = v2; indices[idx++] = v3; indices[idx++] = v1;
        }
    }

    glBindVertexArray(g_surface.vao);

    if (numVertices * sizeof(Vertex) > g_surface.vertexBufferSize) {
        g_surface.vertexBufferSize = numVertices * sizeof(Vertex);
        glBindBuffer(GL_ARRAY_BUFFER, g_surface.vbo);
        glBufferData(GL_ARRAY_BUFFER, g_surface.vertexBufferSize, NULL, GL_DYNAMIC_DRAW);
    }

    if (numIndices * sizeof(GLuint) > g_surface.indexBufferSize) {
        g_surface.indexBufferSize = numIndices * sizeof(GLuint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_surface.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_surface.indexBufferSize, NULL, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, g_surface.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * sizeof(Vertex), vdata);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_surface.ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices * sizeof(GLuint), indices);

    glBindVertexArray(0);

    g_surface.numVertices = numVertices;
    g_surface.numIndices  = numIndices;

    free(vdata);
    free(indices);
}
