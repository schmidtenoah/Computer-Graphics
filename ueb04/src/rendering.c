/**
 * @file rendering.c
 * @brief Implementation of rendering system
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "rendering.h"
#include "input.h"
#include "model.h"
#include "shader.h"
#include "physics.h"

#define NEAR_PLANE 0.01f
#define FAR_PLANE 100.0f
#define FOV_Y 60.0f
#define ROOM_SIZE 10.0f

////////////////////////    LOCAL    ////////////////////////////

typedef struct {
    ivec2 screenRes;
    float aspect;
} RenderingData;

static RenderingData g_renderingData;

/**
 * Updates camera view matrix
 */
static void updateCamera(InputData *data) {
    camera_getPosition(data->cam.data, data->cam.pos);
    camera_getFront(data->cam.data, data->cam.dir);
    scene_look(data->cam.pos, data->cam.dir, GLM_YUP);
}


/**
 * Draws entire room (floor, ceiling, walls)
 */
static void drawRoom(InputData *data) {
    debug_pushRenderScope("Room");
    scene_pushMatrix();

    glCullFace(GL_FRONT);
    float s = data->rendering.roomSize;
    scene_scale(s, s, s);
    model_drawTextured(MODEL_CUBE, data->rendering.texOrder1);
    glCullFace(GL_BACK);

    scene_popMatrix();
    debug_popRenderScope();
}

////////////////////////    PUBLIC    ////////////////////////////

void rendering_init(void) {
    memset(&g_renderingData, 0, sizeof(RenderingData));

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    shader_load();
}

void rendering_draw(void) {
    InputData *data = getInputData();

    if (data->showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    glEnable(GL_DEPTH_TEST);

    debug_pushRenderScope("Scene");
    scene_pushMatrix();

    updateCamera(data);
    drawRoom(data);
    physics_drawSpheres();

    scene_popMatrix();
    debug_popRenderScope();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void rendering_cleanup(void) {
    shader_cleanup();
    camera_deleteCamera(&getInputData()->cam.data);
}

void rendering_resize(int width, int height) {
    g_renderingData.screenRes[0] = width;
    g_renderingData.screenRes[1] = height;
    g_renderingData.aspect = (float)width / height;

    scene_perspective(FOV_Y, g_renderingData.aspect, NEAR_PLANE, FAR_PLANE);
}