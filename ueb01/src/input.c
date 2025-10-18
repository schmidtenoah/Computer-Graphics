/**
 * 
 */

#include "input.h"
#include "rendering.h"
#include "shader.h"
#include "utils.h"
#include "logic.h"

// LOCAL

static InputData input;

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

static void input_frameBufferSizeEvent(ProgContext ctx, int width, int height) {
    NK_UNUSED(ctx);
    rendering_resize(width, height, getInputData()->curve.buttonCount);
}

static void input_mouseButtonEvent(ProgContext ctx, int button, int action, int mods) {
    NK_UNUSED(ctx);
    NK_UNUSED(mods);

    InputData* data = getInputData();
    data->mouse.button = button;
    data->mouse.action = action;
}

static void input_mouseMoveEvent(ProgContext ctx, double x, double y) {
    NK_UNUSED(ctx);

    InputData* data = getInputData();
    data->mouse.xPos = (float) x;
    data->mouse.yPos = (float) y;
}


// PUBLIC

void input_init(ProgContext ctx) {
    NK_UNUSED(ctx);

    input.isFullscreen = false;
    input.showHelp = false;
    input.showMenu = true;
    input.showWireframe = false;
    input.mouse.button = GLFW_KEY_UNKNOWN;
    input.mouse.xPos = 0;
    input.mouse.yPos = 0;
    input.curve.resolution = 0.02f;
    input.curve.width = 2.0f;
    input.curve.drawPolygon = false;
    input.curve.drawConvexHull = false;
    input.game.isFlying = false;
    input.game.showColliders = false;
    input.curve.showNormals = false;
    input.curve.curveEval = utils_bSplineUniformGlobal;
}

InputData* getInputData(void) {
    return &input;
}

void input_registerCallbacks(ProgContext ctx) {
    window_setKeyboardCallback(ctx, input_keyEvent);
    window_setMouseButtonCallback(ctx, input_mouseButtonEvent);
    window_setMouseMovementCallback(ctx, input_mouseMoveEvent);
    window_setFramebufferSizeCallback(ctx, input_frameBufferSizeEvent);
}
