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
#include "model.h"

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

/**
 * Transforms the given vec3 from world to view space based on 
 * the current Stack.
 * @param vec The vec3 to be transformed.
 * @param dest The output for the view space vec3.
 * @param isPos If the given vec3 is a position or a direction.
 */
static void worldToView(vec3 vec, vec3 dest, bool isPos) {
    mat4 vm;
    scene_getMV(vm);

    vec4 vecWS = {vec[0], vec[1], vec[2], isPos};
    glm_mat4_mulv(vm, vecWS, vecWS);
    glm_vec3_copy((vec3) {vecWS[0], vecWS[1], vecWS[2]}, dest);
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

void shader_setMVP(mat4 *viewMat, mat4 *modelviewMat, const Material *m) {
    shader_useShader(modelShader);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(modelShader, "u_mvpMatrix", &mat);
    shader_setMat4(modelShader, "u_viewMatrix", viewMat);
    shader_setMat4(modelShader, "u_modelviewMatrix", modelviewMat);

    bool useMat = m != NULL;
    shader_setBool(modelShader, "u_useMaterial", useMat);
    if (useMat) {
        shader_setVec3(modelShader, "u_material.ambient", (vec3*)m->ambient);
        shader_setVec3(modelShader, "u_material.diffuse", (vec3*)m->diffuse);
        shader_setVec3(modelShader, "u_material.specular", (vec3*)m->specular);
        shader_setVec3(modelShader, "u_material.emission", (vec3*)m->emission);
        shader_setFloat(modelShader, "u_material.shininess", m->shininess);
        shader_setFloat(modelShader, "u_material.alpha", m->alpha);
    }
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

void shader_setTexture(GLuint textureId, bool useTexture) {
    shader_useShader(modelShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    shader_setInt(modelShader, "u_texture", 0);
    shader_setBool(modelShader, "u_useTexture", useTexture);
}

void shader_setCamPos(vec3 camPosWS) {
    shader_useShader(modelShader);
    vec3 camPosVS = {0};
    worldToView(camPosWS, camPosVS, true);
    shader_setVec3(modelShader, "u_camPosVS", (vec3*)camPosVS);
}

void shader_setPointLight(vec3 color, vec3 posWS, vec3 falloff, bool enabled, float ambientFactor) {
    shader_useShader(modelShader);
    vec3 posVS = {0};
    worldToView(posWS, posVS, true);
    shader_setVec3(modelShader, "u_pointLight.posVS", &posVS);
    shader_setVec3(modelShader, "u_pointLight.color", (vec3*)color);
    shader_setVec3(modelShader, "u_pointLight.falloff", (vec3*)falloff);
    shader_setBool(modelShader, "u_pointLight.enabled", enabled);
    shader_setFloat(modelShader, "u_pointLight.ambientFactor", ambientFactor);
}