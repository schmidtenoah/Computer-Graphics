/**
 * @file gui.c
 * @brief Implementation of GUI components.
 *
 * Manages the rendering of the help overlay, settings menu
 * and start button.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "gui.h"
#include "input.h"
#include "logic.h"
#include "utils.h"
#include "physics.h"

#define GUI_WINDOW_HELP "window_help"
#define GUI_WINDOW_MENU "window_menu"

////////////////////////     LOCAL    ////////////////////////////

/* Booleans to toggle between spline and bezier */
static bool g_showSpline = true;
static bool g_showBezier = false;

/**
 * Constant array for help messages and their correspondant button.
 */
static const GuiHelpLine help[] = {
    {"Quit Programm", "ESC"},
    {"Toggle Help", "F1"},
    {"Toggle Fullscreen", "F2"},
    {"Toggle Wireframe", "F3"},
    {"Toggle Menu", "F4"},
    {"Reload Shaders", "R"}, 
    {"Height Functions", "1-7"},
    {"Pause", "P"},
    {"Normals", "N"},
    {"Camera Flight", "C"},
    {"Toggle Flight Path", "V"},
    {"Select CP", "Left/Right"},
    {"Adjust Height", "Up/Down"}
};

/**
 * Renders the help overlay window showing keyboard shortcuts.
 * Only displays if input->showHelp is true.
 *
 * @param ctx Program context
 * @param input Pointer to input data containing GUI state
 */
static void gui_renderHelp(ProgContext ctx, InputData* input) {
    if (!(input->showHelp)) {
        return;
    }

    int w, h;
    window_getRealSize(ctx, &w, &h);
    float width = w * 0.25f;
    float height = h * 0.5f;
    float x = width * 1.5f;
    float y = height * 0.5f;

    input->showHelp = gui_widgetHelp(ctx, help, NK_LEN(help), nk_rect(x, y, width, height));
}

/**
 * Renders the main settings menu window with collapsible sections.
 * Sections:
 * - General (help, fullscreen, pause),
 * - Visual (wireframe),
 * - Curve (spline/bezier, polygon, convex hull, normals width, resolution),
 * - Game (level info, restart/skip, start, colliders, airplane speed)
 * Only displays if input->showMenu is true.
 *
 * @param ctx Program context
 * @param input Pointer to input data containing GUI state
 */
static void gui_renderMenu(ProgContext ctx, InputData* input) {
    if (!(input->showMenu)) {
        return;
    }

    int w;
    window_getRealSize(ctx, &w, NULL);
    float height = 0.7f * w;

    if (gui_beginTitled(ctx, GUI_WINDOW_MENU, "Settings", 
        nk_rect(15, 15, 200, height),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
        NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        if (gui_treePush(ctx, NK_TREE_TAB, "General", NK_MAXIMIZED)){
            gui_layoutRowDynamic(ctx, 20, 2);

            if (gui_button(ctx, "Help")) {
                input->showHelp = !input->showHelp;
            }

            if (gui_button(ctx, input->isFullscreen ? "Window" : "Fullscreen")){
                input->isFullscreen = !input->isFullscreen;
                window_setFullscreen(ctx, input->isFullscreen);
            }

            gui_layoutRowDynamic(ctx, 20, 1);
            if (gui_button(ctx, input->paused ? "unpause" : "pause")) {
                input->paused = !input->paused;
            }

            gui_checkbox(ctx, "Wireframe", &input->showWireframe);

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "PHYSICS", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);

            if (gui_button(ctx, "reset")) {
                physics_init();
            }

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Light", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);

            gui_checkbox(ctx, "enabled", &input->pointLight.enabled);
            gui_checkbox(ctx, "visualize", &input->pointLight.visualize);
            gui_widgetColor3(ctx, "color", input->pointLight.color);
            gui_propertyFloat(ctx, "falloff constant", 0.0f, &input->pointLight.falloff[0], 10.0f, 0.0001f, 0.01f);
            gui_propertyFloat(ctx, "falloff linear", 0.0f, &input->pointLight.falloff[1], 10.0f, 0.0001f, 0.01f);
            gui_propertyFloat(ctx, "falloff quadratic", 0.0f, &input->pointLight.falloff[2], 10.0f, 0.0001f, 0.01f);
            gui_propertyFloat(ctx, "ambient factor", 0.0f, &input->pointLight.ambientFactor, 1.0f, 0.0001f, 0.1f);
            gui_propertyFloat(ctx, "speed", 0.0f, &input->pointLight.speed, 10.0f, 0.01f, 0.1f);
            gui_propertyFloat(ctx, "radius", 0.001f, &input->pointLight.rotationRadius, 10.0f, 0.0001f, 0.01f);
            gui_widgetVec3(ctx, "center", input->pointLight.center, 10.0f, 0.001f, 0.01f);

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Surface", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);

            int oldDim = input->surface.dimension;
            gui_propertyInt(ctx, "dim", 4, &input->surface.dimension, 500, 1, 0.1f);
            input->surface.dimensionChanged = oldDim != input->surface.dimension;

            int oldRes = input->surface.resolution;
            gui_propertyInt(ctx, "res", 2, &input->surface.resolution, 500, 1, 0.1f);
            input->surface.resolutionChanged = oldRes != input->surface.resolution;

            float oldOffset = input->surface.controlPointOffset;
            gui_propertyFloat(ctx, "offset", 0, &input->surface.controlPointOffset, 2, 0.001f, 0.01f);
            input->surface.offsetChanged = !glm_eq(oldOffset, input->surface.controlPointOffset);

            gui_checkbox(ctx, "Control Points", &input->surface.showControlPoints);
            gui_checkbox(ctx, "Surface", &input->surface.showSurface);
            gui_checkbox(ctx, "Normals", &input->showNormals);
            gui_checkbox(ctx, "Use Texture (T)", &input->surface.useTexture);
            
            if (input->surface.useTexture) {
                gui_propertyInt(ctx, "Texture", 0, &input->surface.currentTextureIndex, 2, 1, 1);
                
                float oldTiling = input->surface.textureTiling;
                gui_propertyFloat(ctx, "Tiling", 0.5f, &input->surface.textureTiling, 20.0f, 0.1f, 0.1f);
                if (!glm_eq(oldTiling, input->surface.textureTiling)) {
                    input->surface.resolutionChanged = true;
                }
            }

            if (gui_button(ctx, "print polynomials")) {
                logic_printPolynomials();
            }


            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Selection", NK_MAXIMIZED)){
            gui_layoutRowDynamic(ctx, 20, 1);

            char infoStr[15];
            snprintf(infoStr, 14, "Selected: %d", input->selection.selectedCp);
            gui_labelColor(ctx, infoStr, NK_TEXT_CENTERED, (ivec3){100, 100, 255});

            gui_layoutRowDynamic(ctx, 20, 2);
            if (gui_button(ctx, "+")) {
                input->selection.selectedCp = (input->selection.selectedCp + input->selection.skipCnt) 
                % input->surface.controlPoints.size;
            }
            if (gui_button(ctx, "-")) {
                input->selection.selectedCp = (input->selection.selectedCp - input->selection.skipCnt) 
                % input->surface.controlPoints.size;
            }

            gui_layoutRowDynamic(ctx, 20, 1);
            if (gui_button(ctx, "jump to center")) {
                input->selection.selectedCp = (int) ((float) input->surface.controlPoints.size * 0.5f);
            }

            gui_propertyInt(ctx, "skip count", 1, &input->selection.skipCnt, 200, 1, 0.1f);
            gui_propertyFloat(ctx, "height change", 0.01f, &input->selection.selectedYChange, 2, 0.01f, 0.01f);
       
            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Camera Flight", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);

            if (gui_button(ctx, input->cam.isFlying ? "Flying..." : "Start Flight (C)")) {
                if (!input->cam.isFlying) {
                    input->cam.isFlying = true;
                    input->cam.flight.t = 0.0f;
                }
            }

            gui_checkbox(ctx, "Show Path (V)", &input->cam.flight.showPath);

            gui_propertyFloat(ctx, "duration", 1.0f, &input->cam.flight.duration, 20.0f, 0.1f, 0.1f);

            gui_treePop(ctx);
        }
    }
    gui_end(ctx);
}

/**
 * Renders camera flight controls at bottom-right corner of the screen.
 *
 * @param ctx Program context
 * @param input Pointer to input data
 */
static void gui_renderCameraControls(ProgContext ctx, InputData* input) {
    int w, h;
    window_getRealSize(ctx, &w, &h);

    if (gui_begin(ctx, "camera_controls", nk_rect((float) w - 150, (float) h - 60, 150, 60), 
        NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) 
    {
        gui_layoutRowDynamic(ctx, 25, 1);
        
        if (!input->cam.isFlying) {
            if (gui_button(ctx, "Start Flight (C)")) {
                input->cam.isFlying = true;
                input->cam.flight.t = 0.0f;
            }
        } else {
            char label[32];
            snprintf(label, sizeof(label), "Flying... %.0f%%", input->cam.flight.t * 100.0f);
            gui_label(ctx, label, NK_TEXT_CENTERED);
        }
        
        gui_layoutRowDynamic(ctx, 25, 1);
        if (gui_button(ctx, input->cam.flight.showPath ? "Hide Path" : "Show Path")) {
            input->cam.flight.showPath = !input->cam.flight.showPath;
        }
    }

    gui_end(ctx);
}

////////////////////////     PUBLIC    ////////////////////////////

void gui_renderContent(ProgContext ctx)
{
    InputData* input = getInputData();
    gui_renderHelp(ctx, input);
    gui_renderMenu(ctx, input);
    gui_renderCameraControls(ctx, input);
}
