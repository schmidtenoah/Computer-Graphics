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

#include <fhwcg/fhwcg.h>

#include "input.h"
#include "rendering.h"
#include "shader.h"
#include "utils.h"
#include "logic.h"

#define CAM_START_POS VEC3(0, 2, 1.8f)
#define CAM_SPEED 0.5f
#define CAM_FAST_SPEED (CAM_SPEED * 3.0f)
#define CAM_SENSITIVITY 0.1f
#define CAM_YAW -90
#define CAM_PITCH -50

#define SURFACE_START_DIM 4
#define CONTROL_POINT_OFFSET 0.1f
#define SELECTED_CONTROL_POINT_Y_CHANGE 0.01f

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

    InputData* data = getInputData();
    camera_keyboardCallback(data->cam.data, key, action);

    data->selection.pressingUp = (key == GLFW_KEY_UP) && (action != GLFW_RELEASE);
    data->selection.pressingDown = (key == GLFW_KEY_DOWN) && (action != GLFW_RELEASE);

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

        case GLFW_KEY_N:
            data->showNormals = !data->showNormals;
            break;

        case GLFW_KEY_KP_ADD:
            data->surface.resolution = (int) glm_clamp(data->curve.resolution - 1, 2, 100);
            data->surface.resolutionChanged = true;
            break;

        case GLFW_KEY_KP_SUBTRACT:
        case GLFW_KEY_MINUS:
            data->surface.resolution = (int) glm_clamp(data->curve.resolution + 1, 2, 100);
            data->surface.resolutionChanged = true;
            break;

        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4:
        case GLFW_KEY_5:
        case GLFW_KEY_6:
        case GLFW_KEY_7:
            utils_applyHeightFunction((HeightFuncType) (key - GLFW_KEY_1));
            break;

        case GLFW_KEY_RIGHT:
            data->selection.selectedCp = (data->selection.selectedCp + data->selection.skipCnt) 
            % data->surface.controlPoints.size;
            break;

         case GLFW_KEY_LEFT:
            data->selection.selectedCp = (data->selection.selectedCp - data->selection.skipCnt) 
            % data->surface.controlPoints.size;
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
    rendering_resize(width, height);
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
    NK_UNUSED(mods);
    NK_UNUSED(ctx);

    InputData* data = getInputData();
    data->mouse.button = button;
    data->mouse.action = action; // TODO: noch nötig?

    camera_mouseButtonCallback(data->cam.data, button, action);
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
    InputData* data = getInputData();
    data->mouse.xPos = (float) x;
    data->mouse.yPos = (float) y; // TODO: noch nötig?

    camera_mouseMoveCallback(data->cam.data, ctx, (float) x, (float) y);
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
    g_input.curve.buttonsChanged = true;
    g_input.curve.resolutionChanged = true;
    g_input.showNormals = false;

    g_input.cam.data = camera_createCamera(
        ctx, CAM_START_POS, 
        CAM_SPEED, CAM_FAST_SPEED, 
        CAM_SENSITIVITY, CAM_YAW, CAM_PITCH
    );
    camera_getPosition(g_input.cam.data, g_input.cam.pos);
    camera_getFront(g_input.cam.data, g_input.cam.dir);
    g_input.cam.isFlying = false;

    g_input.surface.dimension = SURFACE_START_DIM;
    g_input.surface.resolution = SURFACE_START_DIM;
    g_input.surface.dimensionChanged = true;
    g_input.surface.resolutionChanged = true;
    g_input.surface.offsetChanged = true;
    g_input.surface.showControlPoints = true;
    g_input.surface.showSurface = true;
    g_input.surface.controlPointOffset = CONTROL_POINT_OFFSET;
    vec3arr_init(&g_input.surface.controlPoints);

    g_input.selection.selectedCp = 0;
    g_input.selection.skipCnt = 1;
    g_input.selection.selectedYChange = SELECTED_CONTROL_POINT_Y_CHANGE;
    g_input.selection.pressingDown = false;
    g_input.selection.pressingUp = false;
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
