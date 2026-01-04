/**
 * @file shader.c
 * @brief Implementation of shader loading and uniforms.
 *
 * Handles loading shader files, compilation, linking, and reloading.
 * Manages three shader programs:
 * - simple (colored rendering),
 * - model (lighting and materials),
 * - normal (normal vector visualization with geometry shader).
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

 #include <fhwcg/fhwcg.h>

#include "shader.h"
#include "rendering.h"
#include "model.h"

#define NORMAL_COLOR ((vec3) {1, 0, 0})
#define NORMAL_LENGTH 0.1f

////////////////////////    LOCAL    ////////////////////////////

// Shaders & Material struct
static Shader *pVecsShader, *simpleShader, *dropShadowShader, *textureShader;
struct Material;

/**
 * Helper function to delete a shader and set pointer to NULL.
 *
 * @param s Pointer to shader pointer to clean up
 */
static void cleanup(Shader *s) {
    if (s) {
        shader_deleteShader(&s);
        s = NULL;
    }
}

/**
 * Creates and compiles the particle vectors shader.
 * Uses vertex, geometry, and fragment shaders.
 * @return Pointer to the compiled shader.
 */
static Shader* createParticleVecsShader(void) {
    Shader* shader = shader_createShader();
    shader_attachShaderFile(shader, GL_VERTEX_SHADER,   RESOURCE_PATH "shader/particleVecs/particleVecs.vert");
    shader_attachShaderFile(shader, GL_GEOMETRY_SHADER, RESOURCE_PATH "shader/particleVecs/particleVecs.geom");
    shader_attachShaderFile(shader, GL_FRAGMENT_SHADER, RESOURCE_PATH "shader/particleVecs/particleVecs.frag");

    shader_buildShader("Particle Vectors", shader);
    return shader;
}
////////////////////////    PUBLIC    ////////////////////////////

void shader_cleanup(void) {
    cleanup(pVecsShader);
    cleanup(simpleShader);
    cleanup(textureShader);
    cleanup(dropShadowShader);
}

void shader_load(void) {
    Shader *newShader = NULL;

    newShader = shader_createVeFrShader(
        "simple",
        RESOURCE_PATH "shader/simple/simple.vert",
        RESOURCE_PATH "shader/simple/simple.frag"
    );
    if (newShader) {
        cleanup(simpleShader);
        simpleShader = newShader;
    }

    newShader = shader_createVeFrShader(
        "drop shadow",
        RESOURCE_PATH "shader/dropShadow/dropShadow.vert",
        RESOURCE_PATH "shader/dropShadow/dropShadow.frag"
    );
    if (newShader) {
        cleanup(dropShadowShader);
        dropShadowShader = newShader;
    }

    newShader = createParticleVecsShader();
    if (newShader) {
        cleanup(pVecsShader);
        pVecsShader = newShader;
    }

    newShader = shader_createVeFrShader(
        "texture",
        RESOURCE_PATH "shader/textured/textured.vert",
        RESOURCE_PATH "shader/textured/textured.frag"
    );
    if (newShader) {
        cleanup(textureShader);
        textureShader = newShader;
    }
}

void shader_setColor(vec3 color) {
    shader_useShader(simpleShader);
    shader_setVec3(simpleShader, "u_color", (vec3*) color);
}

void shader_setSimpleMVP(bool drawInstanced) {
    shader_useShader(simpleShader);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(simpleShader, "u_mvpMatrix", &mat);
    shader_setBool(simpleShader, "u_drawInstanced", drawInstanced);
}

void shader_setSimpleInstanceData(vec3 scale, int leaderIdx) {
    shader_useShader(simpleShader);
    shader_setVec3(simpleShader, "u_localScale", (vec3*) scale);
    shader_setInt(simpleShader, "u_leaderIdx", leaderIdx);
}

void shader_setParticleVisData(vec3 scale) {
    shader_useShader(pVecsShader);
    
    shader_setVec3(pVecsShader, "u_localScale", (vec3*) scale);
    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(pVecsShader, "u_mvpMatrix", &mat);
}

void shader_setDropShadowData(vec3 scale, int leaderIdx, bool drawInstanced, float groundHeight) {
    shader_useShader(dropShadowShader);

    shader_setVec3(dropShadowShader, "u_localScale", (vec3*) scale);
    shader_setInt(dropShadowShader, "u_leaderIdx", leaderIdx);
    shader_setFloat(dropShadowShader, "u_groundHeight", groundHeight);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(dropShadowShader, "u_mvpMatrix", &mat);
    shader_setBool(dropShadowShader, "u_drawInstanced", drawInstanced);
}

Shader* shader_getTextureShader(void) {
    return textureShader;
}
