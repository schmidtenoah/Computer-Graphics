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
#define ROOM_SIZE 5.0f

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
 * Draws floor (bottom face, Y = -ROOM_SIZE/2) - YELLOW
 */
static void drawFloor(void) {
    scene_pushMatrix();

    scene_translate(0, -ROOM_SIZE / 2.0f, 0);
    scene_rotate(90, 1, 0, 0);
    scene_scale(ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 1.0f);

    shader_setColor((vec3){1.0f, 1.0f, 0.0f}); // Yellow
    model_drawSimple(MODEL_CUBE);

    scene_popMatrix();
}

/**
 * Draws ceiling (top face, Y = +ROOM_SIZE/2) - BLUE
 */
static void drawCeiling(void) {
    scene_pushMatrix();

    scene_translate(0, ROOM_SIZE / 2.0f, 0);
    scene_rotate(-90, 1, 0, 0);
    scene_scale(ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 1.0f);

    shader_setColor((vec3){0.3f, 0.3f, 0.8f}); // Blue
    model_drawSimple(MODEL_CUBE);

    scene_popMatrix();
}

/**
 * Draws wall at specified position and rotation - BLUE
 */
static void drawWall(float posX, float posY, float posZ,
                     float rotAngle, float rotX, float rotY, float rotZ) {
    scene_pushMatrix();

    scene_translate(posX, posY, posZ);
    scene_rotate(rotAngle, rotX, rotY, rotZ);
    scene_scale(ROOM_SIZE / 2.0f, ROOM_SIZE / 2.0f, 1.0f);

    shader_setColor((vec3){0.3f, 0.3f, 0.8f}); // Blue
    model_drawSimple(MODEL_CUBE);

    scene_popMatrix();
}

/**
 * Draws all four walls - BLUE
 */
static void drawWalls(void) {
    // Front wall (Z = -ROOM_SIZE/2)
    drawWall(0, 0, -ROOM_SIZE / 2.0f, 0, 1, 0, 0);

    // Back wall (Z = +ROOM_SIZE/2)
    drawWall(0, 0, ROOM_SIZE / 2.0f, 180, 0, 1, 0);

    // Left wall (X = -ROOM_SIZE/2)
    drawWall(-ROOM_SIZE / 2.0f, 0, 0, 90, 0, 1, 0);

    // Right wall (X = +ROOM_SIZE/2)
    drawWall(ROOM_SIZE / 2.0f, 0, 0, -90, 0, 1, 0);
}

/**
 * Draws entire room (floor, ceiling, walls)
 */
static void drawRoom(void) {
    debug_pushRenderScope("Room");

    drawFloor();
    drawCeiling();
    drawWalls();

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

    debug_pushRenderScope("Scene");
    scene_pushMatrix();

    updateCamera(data);
    drawRoom();
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