/**
 * @file shader.c
 * @brief Implementation of shader loading and uniforms.
 *
 * Handles loading shader files, compilation, linking, and reloading.
 * Manages three shader programs:
 * - simple (colored rendering),
 * - gradient (background),
 * - normalPoint (normal vector visualization with geometry shader).
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

 #include <fhwcg/fhwcg.h>

#include "shader.h"
#include "rendering.h"

#define NORMAL_COLOR ((vec3) {1, 0, 0})
#define NORMAL_LENGTH 0.1f

////////////////////////    LOCAL    ////////////////////////////


static Shader *modelShader, *simpleShader, *normalShader;

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

////////////////////////    PUBLIC    ////////////////////////////

void shader_cleanup(void) {
    cleanup(modelShader);
    cleanup(simpleShader);
    cleanup(normalShader);
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
        "model", 
        RESOURCE_PATH "shader/model/model.vert", 
        RESOURCE_PATH "shader/model/model.frag"
    );
    if (newShader) {
        cleanup(modelShader);
        modelShader = newShader;
    }

    newShader = shader_createNormalsShader(FHWCG_SHADER_PATH);
    if (newShader) {
        cleanup(normalShader);
        normalShader = newShader;

        shader_useShader(normalShader);
        shader_setFloat(normalShader, "u_normalLength", NORMAL_LENGTH);
        shader_setVec3(normalShader, "u_color", &NORMAL_COLOR);
    }
}

void shader_setMVP(mat4 *viewMat, mat4 *modelviewMat) {
    shader_useShader(modelShader);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(modelShader, "u_mvpMatrix", &mat);
    shader_setMat4(modelShader, "u_viewMatrix", viewMat);
    shader_setMat4(modelShader, "u_modelviewMatrix", modelviewMat);
}

void shader_setColor(vec3 color) {
    shader_useShader(simpleShader);
    shader_setVec3(simpleShader, "u_color", (vec3*) color);
}

void shader_setNormals(void) {
    shader_useShader(normalShader);
    mat4 mat;
    scene_getMV(mat);
    shader_setMat4(normalShader, "u_modelViewMatrix", &mat);
    scene_getN(mat);
    shader_setMat4(normalShader, "u_normalMatrix", &mat);
    scene_getP(mat);
    shader_setMat4(normalShader, "u_projMatrix", &mat);
}

void shader_setSimpleMVP(void) {
    shader_useShader(simpleShader);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(simpleShader, "u_mvpMatrix", &mat);
}
