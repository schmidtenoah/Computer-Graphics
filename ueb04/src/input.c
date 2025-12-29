/**
 * @file input.c
 * @brief Implementation of input handling
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "input.h"
#include "rendering.h"
#include "shader.h"
#include "physics.h"

#define CAM_SPEED 2.0f
#define CAM_FAST_SPEED (CAM_SPEED * 6.0f)
#define CAM_SENSITIVITY 0.1f
#define CAM_YAW -90.0f
#define CAM_PITCH -20.0f

#define SIMULATION_SPEED 1.0f
#define SIMULATION_FPS 120.0f

#define GAUSSIAN_CONST 60.0f
#define LEADER_KV 5.0f

#define CENTER_MOVE_SPEED 0.2f

////////////////////////    LOCAL    ////////////////////////////

/** Global input state */
static InputData g_input = {0};

/**
 * Keyboard callback
 */
static void input_keyEvent(ProgContext ctx, int key, int action, int mods) {
    NK_UNUSED(mods);

    InputData *data = getInputData();
    camera_keyboardCallback(data->cam.data, key, action);

    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    // Arrow keys for center movement (when in CENTER mode)
    if (data->particles.targetMode == TM_CENTER) {
        vec3 delta = {0, 0, 0};
        switch (key) {
            case GLFW_KEY_LEFT:
                delta[0] = -CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
            case GLFW_KEY_RIGHT:
                delta[0] = CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
            case GLFW_KEY_UP:
                delta[2] = -CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
            case GLFW_KEY_DOWN:
                delta[2] = CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
            case GLFW_KEY_PAGE_UP:
                delta[1] = CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
            case GLFW_KEY_PAGE_DOWN:
                delta[1] = -CENTER_MOVE_SPEED;
                physics_moveCenterManual(delta);
                return;
        }
    }

    if (action != GLFW_PRESS) {
        return;
    }

    switch (key) {
        case GLFW_KEY_ESCAPE:
            window_shouldCloseWindow(ctx);
            break;

        case GLFW_KEY_F1:
            data->showHelp = !data->showHelp;
            break;

        case GLFW_KEY_F2:
            data->isFullscreen = !data->isFullscreen;
            window_setFullscreen(ctx, data->isFullscreen);
            break;

        case GLFW_KEY_F3:
            data->showWireframe = !data->showWireframe;
            break;

        case GLFW_KEY_F4:
            data->showMenu = !data->showMenu;
            break;

        case GLFW_KEY_R:
            shader_load();
            break;

        case GLFW_KEY_P:
            data->paused = !data->paused;
            break;

        case GLFW_KEY_T:
            data->rendering.texOrder1 = !data->rendering.texOrder1;
            break;

        case GLFW_KEY_L:
            if (data->particles.targetMode == TM_LEADER) {
                physics_setNewLeader();
            }
            break;

        default:
            break;
    }
}

/**
 * Framebuffer resize callback
 */
static void input_frameBufferSizeEvent(ProgContext ctx, int width, int height) {
    NK_UNUSED(ctx);
    rendering_resize(width, height);
}

/**
 * Mouse button callback
 */
static void input_mouseButtonEvent(ProgContext ctx, int button, int action, int mods) {
    NK_UNUSED(mods);
    NK_UNUSED(ctx);

    InputData *data = getInputData();
    camera_mouseButtonCallback(data->cam.data, button, action);
}

/**
 * Mouse movement callback
 */
static void input_mouseMoveEvent(ProgContext ctx, double x, double y) {
    InputData *data = getInputData();
    camera_mouseMoveCallback(data->cam.data, ctx, (float)x, (float)y);
}

////////////////////////    PUBLIC    ////////////////////////////

void input_init(ProgContext ctx) {
    glLineWidth(0.5f);
    g_input.isFullscreen = false;
    g_input.showHelp = false;
    g_input.showMenu = true;
    g_input.showWireframe = false;
    g_input.paused = false;

    vec3 startPos = {0.0f, 1.5f, 3.0f};
    g_input.cam.data = camera_createCamera(
        ctx, startPos,
        CAM_SPEED, CAM_FAST_SPEED,
        CAM_SENSITIVITY, CAM_YAW, CAM_PITCH
    );
    camera_getPosition(g_input.cam.data, g_input.cam.pos);
    camera_getFront(g_input.cam.data, g_input.cam.dir);

    g_input.rendering.texOrder1 = false;
    g_input.rendering.roomSize = 10.0f;

    g_input.physics.fixedDt = 1.0f / SIMULATION_FPS;
    g_input.physics.sphereRadius = 0.5f;
    g_input.physics.dtAccumulator = 0.0f;
    g_input.physics.simulationSpeed = SIMULATION_SPEED;
    g_input.physics.roomForce = 10.0f;

    g_input.particles.count = START_NUM_PARTICLES;
    g_input.particles.gaussianConst = GAUSSIAN_CONST;
    g_input.particles.leaderKv = LEADER_KV;
    g_input.particles.sphereVis = SV_SPHERE;
    g_input.particles.targetMode = TM_SPHERES;
    g_input.particles.visVectors = true;
    g_input.particles.leaderIdx = 0;
}

InputData* getInputData(void) {
    return &g_input;
}

void input_registerCallbacks(ProgContext ctx) {
    window_setKeyboardCallback(ctx, input_keyEvent);
    window_setMouseButtonCallback(ctx, input_mouseButtonEvent);
    window_setMouseMovementCallback(ctx, input_mouseMoveEvent);
    window_setFramebufferSizeCallback(ctx, input_frameBufferSizeEvent);
}