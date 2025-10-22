#include "shader.h"
#include "rendering.h"

#define NORMAL_COLOR ((vec3) {1, 1, 0})
#define NORMAL_LENGTH 0.1f

// LOCAL

static Shader *shader = NULL, *normalPointShader = NULL, *gradientShader = NULL;

static void cleanup(Shader *s) {
    if (s) {
        shader_deleteShader(&s);
        s = NULL;
    }
}

// PUBLIC

void shader_cleanup(void) {
    cleanup(shader);
    cleanup(normalPointShader);
    cleanup(gradientShader);
}

void shader_load(void) {
    Shader* newShader = shader_createVeFrShader(
        "Simple", 
        RESOURCE_PATH "shader/simple/simple.vert", 
        RESOURCE_PATH "shader/simple/simple.frag"
    );

    Shader *newGradientShader = shader_createVeFrShader(
        "Gradient",
        RESOURCE_PATH "shader/gradient/gradient.vert",
        RESOURCE_PATH "shader/gradient/gradient.frag"
    );

    if (newShader) {
        cleanup(shader);
        shader = newShader;
    }

    if (newGradientShader) {
        cleanup(gradientShader);
        gradientShader = newGradientShader;
    }

    Shader *newNormalShader = shader_createShader();
    shader_attachShaderFile(newNormalShader, GL_VERTEX_SHADER, RESOURCE_PATH "shader/normalPoint/normalPoint.vert");
    shader_attachShaderFile(newNormalShader, GL_GEOMETRY_SHADER, RESOURCE_PATH "shader/normalPoint/normalPoint.geom");
    shader_attachShaderFile(newNormalShader, GL_FRAGMENT_SHADER, RESOURCE_PATH "shader/normalPoint/normalPoint.frag");

    if (shader_buildShader("NormalStrip", newNormalShader)) {
        cleanup(normalPointShader);
        normalPointShader = newNormalShader;
        shader_useShader(normalPointShader);
        shader_setFloat(normalPointShader, "u_normalLength", NORMAL_LENGTH);
        shader_setVec3(normalPointShader, "u_color", &NORMAL_COLOR);
    }
}

void shader_setMVP(void) {
    shader_useShader(shader);

    mat4 mat;
    scene_getMVP(mat);
    shader_setMat4(shader, "u_mvpMatrix", &mat);
}

void shader_setColor(vec3 color) {
    shader_useShader(shader);
    shader_setVec3(shader, "u_color", (vec3*) color);
}

void shader_setNormals(void) {
    shader_useShader(normalPointShader);
    mat4 mat;
    scene_getMV(mat);
    shader_setMat4(normalPointShader, "u_modelViewMatrix", &mat);
    scene_getN(mat);
    shader_setMat4(normalPointShader, "u_normalMatrix", &mat);
    scene_getP(mat);
    shader_setMat4(normalPointShader, "u_projMatrix", &mat);
}

void shader_renderGradient(void) {
    shader_useShader(gradientShader);

    vec3 topColor = {0.2f, 0.3f, 0.6f};
    vec3 bottomColor = {0.6f, 0.7f, 0.9f};
    shader_setVec3(gradientShader, "u_topColor", &topColor);
    shader_setVec3(gradientShader, "u_bottomColor", &bottomColor);
}

