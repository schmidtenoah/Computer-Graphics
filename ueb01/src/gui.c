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
    {"Level Selection", "Num-Keys"},
    {"Pause", "P"},
    {"Start", "S"},
    {"Toggle Curve", "B"},
    {"Normals", "N"},
    {"Convex Hull", "C"}
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
static void gui_renderMenu(ProgContext ctx, InputData* input)
{
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

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Visual", NK_MINIMIZED)) {
            // Wireframe
            gui_layoutRowDynamic(ctx, 25, 1);
            gui_checkbox(ctx, "Wireframe", &input->showWireframe);

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Curve", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);
            if (!input->game.isFlying) {
                if (gui_checkbox(ctx, "Spline", &g_showSpline)) {
                    g_showBezier = !g_showSpline;
                    input->curve.curveEval = g_showSpline ? utils_evalSpline : utils_evalBezier;
                }
                if (gui_checkbox(ctx, "Bezier", &g_showBezier)) {
                    if (input->curve.buttonCount != 4) {
                        g_showBezier = !g_showBezier;
                    } else {
                        g_showSpline = !g_showBezier;
                        input->curve.curveEval = g_showSpline ? utils_evalSpline : utils_evalBezier;
                        input->curve.buttonsChanged = true;
                        input->curve.resolutionChanged = true;
                    }
                }
            }
            gui_checkbox(ctx, "Polygon", &input->curve.drawPolygon);
            gui_checkbox(ctx, "Convex Hull", &input->curve.drawConvexHull);
            gui_checkbox(ctx, "Normals", &input->curve.showNormals);

            gui_propertyFloat(ctx, "width", 0.01f, &input->curve.width, 20.0f, 0.001f, 0.5f);

            float newRes = input->curve.resolution;
            gui_propertyFloat(ctx, "resolution", 0.0002f, &newRes, 0.99f, 0.001f, 0.005f);
            if (!glm_eq(newRes, input->curve.resolution)) {
                input->curve.resolution = newRes;
                input->curve.resolutionChanged = true;
            }

            gui_treePop(ctx);
        }

        if (gui_treePush(ctx, NK_TREE_TAB, "Game", NK_MINIMIZED)) {
            gui_layoutRowDynamic(ctx, 25, 1);
            char infoStr[10];
            snprintf(infoStr, 9, "Level: %d", input->game.currentLevel + 1);
            gui_labelColor(ctx, infoStr, NK_TEXT_CENTERED, (ivec3){100, 100, 255});
            
            gui_layoutRowDynamic(ctx, 10, 2);
            if (gui_button(ctx, "=")) {
                logic_restartLevel(input);
            }
            if (gui_button(ctx, ">")) {
                logic_skipLevel(input);
            }

            gui_layoutRowDynamic(ctx, 25, 1);

            if (!input->game.isFlying) {
                if (gui_button(ctx, "Start")) {
                    input->game.isFlying = true;
                }
            }
            
            gui_checkbox(ctx, "Colliders", &input->game.showColliders);

            gui_propertyFloat(ctx, "speed", 0.01f, &input->game.airplane.defaultSpeed, 1.0f, 0.001f, 0.0005f);
    
            gui_treePop(ctx);
        } 
    }
    gui_end(ctx);
}

/**
 * Renders "Start" button at bottom-right corner of the screen.
 * Only visible/clickable if plane isn't flying.
 *
 * @param ctx Program context
 * @param input Pointer to input data to see if plane is flying
 */
static void gui_renderStart(ProgContext ctx, InputData* input) {
    int w, h;
    window_getRealSize(ctx, &w, &h);

    if (gui_begin(ctx, "stats", nk_rect((float) w - 100, (float) h - 30, 100, 30), 
        NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) 
    {
        gui_layoutRowDynamic(ctx, 30, 1);
        if (!input->game.isFlying) {
            if (gui_button(ctx, "Start")) {
                input->game.isFlying = true;
            }
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
    gui_renderStart(ctx, input);
}
