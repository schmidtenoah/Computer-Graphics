/**
 * @file input.c
 * @brief Implementation of input handling
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "input.h"
#include "rendering.h"
#include "shader.h"

#define CAM_SPEED 2.0f
#define CAM_FAST_SPEED (CAM_SPEED * 3.0f)
#define CAM_SENSITIVITY 0.1f
#define CAM_YAW -90.0f
#define CAM_PITCH -20.0f

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
            data->rendering.currentTextureIndex =
                (data->rendering.currentTextureIndex + 1) % 2;
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

    g_input.rendering.currentTextureIndex = 0;

    g_input.physics.gravity = 9.81f;
    g_input.physics.mass = 1.0f;
    g_input.physics.fixedDt = 1.0f / 120.0f;
    g_input.physics.sphereRadius = 0.15f;
    g_input.physics.frictionFactor = 0.998f;
    g_input.physics.bounceDamping = 0.8f;
    g_input.physics.dtAccumulator = 0.0f;
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