/**
 * @file input.c
 * @brief Implementation of input event handling and callbacks.
 *
 * Manages keyboard, mouse, and window resize events.
 * Updates InputData structure based on user input and event to handlers
 * like shader reload, fullscreen toggle, level selection, etc.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "input.h"
#include "rendering.h"
#include "shader.h"
#include "utils.h"
#include "logic.h"

////////////////////////    LOCAL    ////////////////////////////

/** Global application state containing all input, settings, and game data */
static InputData g_input;

/**
 * Callback to handle all keyboard input.
 *
 * @param ctx Program Context
 * @param key Pressed key
 * @param action Corresponding action to pressed key
 * @param mods Unused modifiert keys
 */
static void input_keyEvent(ProgContext ctx, int key, int action, int mods) {
    NK_UNUSED(mods);

    if (action != GLFW_PRESS) {
        return;
    }

    InputData* data = getInputData();
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

        case GLFW_KEY_S:
            data->game.isFlying = true;
            break;

        case GLFW_KEY_B:
            if (data->curve.buttonCount == 4 && !data->game.isFlying) {
                data->curve.curveEval = (data->curve.curveEval == utils_evalSpline) ? utils_evalBezier : utils_evalSpline;
                data->curve.resolutionChanged = true;
                data->curve.buttonsChanged = true;
            }
            break;

        case GLFW_KEY_N:
            data->curve.showNormals = !data->curve.showNormals;
            break;

        case GLFW_KEY_C:
            data->curve.drawConvexHull = !data->curve.drawConvexHull;
            break;

        case GLFW_KEY_KP_ADD:
             data->curve.resolution -= 0.1f;
             data->curve.resolution = glm_clamp(data->curve.resolution, 0.0002f, 0.99f);
             data->curve.resolutionChanged = true;
             break;

        case GLFW_KEY_KP_SUBTRACT:
         case GLFW_KEY_MINUS:
             data->curve.resolution += 0.1f;
             data->curve.resolution = glm_clamp(data->curve.resolution, 0.0002f, 0.99f);
             data->curve.resolutionChanged = true;
             break;

        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
            loadLevel(key - GLFW_KEY_1, data);
            break;
        
        default:
            break;
    }
}

/**
 * Callback for window resize events.
 * Updates rendering viewport and button if window size changes.
 *
 * @param ctx Program context
 * @param width New framebuffer width
 * @param height New framebuffer height
 */
static void input_frameBufferSizeEvent(ProgContext ctx, int width, int height) {
    NK_UNUSED(ctx);
    rendering_resize(width, height, getInputData()->curve.buttonCount);
}

/**
 * Callback for mouse button events (press and release).
 * Stores button and action in InputData
 *
 * @param ctx Program context
 * @param button Mouse button identifier
 * @param action Action performed for button
 * @param mods Unsued modifier keys
 */
static void input_mouseButtonEvent(ProgContext ctx, int button, int action, int mods) {
    NK_UNUSED(ctx);
    NK_UNUSED(mods);

    InputData* data = getInputData();
    data->mouse.button = button;
    data->mouse.action = action;
}

/**
 * Callback for mouse movement events.
 * Updates mouse position in InputData.
 *
 * @param ctx Program context
 * @param x Mouse X coordinate
 * @param y Mouse Y coordinate
 */
static void input_mouseMoveEvent(ProgContext ctx, double x, double y) {
    NK_UNUSED(ctx);

    InputData* data = getInputData();
    data->mouse.xPos = (float) x;
    data->mouse.yPos = (float) y;
}


////////////////////////     PUBLIC    ////////////////////////////

void input_init(ProgContext ctx) {
    NK_UNUSED(ctx);

    g_input.isFullscreen = false;
    g_input.showHelp = false;
    g_input.showMenu = true;
    g_input.showWireframe = false;
    g_input.paused = false;
    g_input.mouse.button = GLFW_KEY_UNKNOWN;
    g_input.mouse.xPos = 0;
    g_input.mouse.yPos = 0;
    g_input.curve.resolution = 0.02f;
    g_input.curve.width = 2.0f;
    g_input.curve.drawPolygon = false;
    g_input.curve.drawConvexHull = false;
    g_input.game.isFlying = false;
    g_input.game.showColliders = false;
    g_input.curve.showNormals = false;
    g_input.curve.curveEval = utils_evalSpline;
    g_input.curve.buttonsChanged = true;
    g_input.curve.resolutionChanged = true;
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
