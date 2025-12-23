/**
 * @file gui.c
 * @brief Implementation of GUI
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "gui.h"
#include "input.h"
#include "physics.h"

#define GUI_WINDOW_HELP "window_help"
#define GUI_WINDOW_MENU "window_menu"

////////////////////////    LOCAL    ////////////////////////////

/** Help lines */
static const GuiHelpLine help[] = {
    {"Quit Program", "ESC"},
    {"Toggle Help", "F1"},
    {"Toggle Fullscreen", "F2"},
    {"Toggle Wireframe", "F3"},
    {"Toggle Menu", "F4"},
    {"Reload Shaders", "R"},
    {"Pause", "P"},
    {"Change Texture", "T"}
};

static const char *visModeDropdown[] = {
    "Sphere", "Line", "Triangle"
};

static const char *targetModeDropdown[] = {
    "Spheres", "Center", "Leader"
};

/**
 * Renders help overlay
 */
static void gui_renderHelp(ProgContext ctx, InputData *input) {
    if (!input->showHelp) {
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
 * Renders physics settings
 */
static void renderPhysics(ProgContext ctx, InputData *input) {
    if (gui_treePush(ctx, NK_TREE_TAB, "Physics", NK_MINIMIZED)) {

        if (gui_treePush(ctx, NK_TREE_NODE, "Spheres", NK_MINIMIZED)) {
            gui_propertyFloat(ctx, "Speed", 0.1f, &input->physics.sphereSpeed, 15.0f, 0.1f, 0.1f);
            gui_propertyFloat(ctx, "radius", 0.01f, &input->physics.sphereRadius, 1.0f, 0.01f, 0.01f);
            if (gui_button(ctx, "toggle wander")) {
                physics_toggleWander();
            }

            gui_treePop(ctx);
        }

        gui_propertyFloat(ctx, "fixed dt", 0.001f, &input->physics.fixedDt, 0.1f, 0.001f, 0.001f);
        gui_propertyFloat(ctx, "sim speed", 0.001f, &input->physics.simulationSpeed, 10.0f, 0.001f, 0.1f);

        gui_treePop(ctx);
    }
}

/**
 * Renders rendering settings
 */
static void renderSettings(ProgContext ctx, InputData *input) {
    if (gui_treePush(ctx, NK_TREE_TAB, "Rendering", NK_MINIMIZED)) {
        gui_layoutRowDynamic(ctx, 25, 1);

        gui_checkbox(ctx, "Wireframe", &input->showWireframe);
        gui_checkbox(ctx, "Texture Order", &input->rendering.texOrder1);
        gui_propertyFloat(ctx, "Room Size", 0.1f, &input->rendering.roomSize, 25.0f, 0.1f, 0.05f);

        gui_treePop(ctx);
    }

    if (gui_treePush(ctx, NK_TREE_TAB, "Particles", NK_MINIMIZED)) {
        if (gui_button(ctx, "spheres wander")) {
            physics_toggleWander();
        }

        gui_propertyFloat(ctx, "Gaussian Const", 30.0f, &input->particles.gaussianConst, 150.0f, 0.1f, 0.05f);
        if (input->particles.targetMode == TM_LEADER) {
            gui_propertyFloat(ctx, "LeaderKv", 2.0f, &input->particles.leaderKv, 10.0f, 0.01f, 0.05f);
        }

        int count = input->particles.count;
        gui_propertyInt(ctx, "particles", 0, &count, 10000, 1, 0.1f);
        if (count != input->particles.count) {
            physics_updateParticleCount(count);
        }

        gui_layoutRowDynamic(ctx, 25, 2);
        gui_label(ctx, "Visual:", NK_TEXT_LEFT);
        input->particles.sphereVis = gui_dropdown(ctx, visModeDropdown, NK_LEN(visModeDropdown), 
            input->particles.sphereVis, 20, nk_vec2(200, 200)
        );

        gui_label(ctx, "Target:", NK_TEXT_LEFT);
        input->particles.targetMode = gui_dropdown(ctx, targetModeDropdown, NK_LEN(targetModeDropdown), 
            input->particles.targetMode, 20, nk_vec2(200, 200)
        );

        gui_layoutRowDynamic(ctx, 25, 1);

        gui_treePop(ctx);
    }
}

/**
 * Renders general controls
 */
static void renderGeneral(ProgContext ctx, InputData *input) {
    if (gui_treePush(ctx, NK_TREE_TAB, "General", NK_MINIMIZED)) {

        if (gui_button(ctx, "Help")) {
            input->showHelp = !input->showHelp;
        }

        if (gui_button(ctx, input->isFullscreen ? "Window" : "Fullscreen")) {
            input->isFullscreen = !input->isFullscreen;
            window_setFullscreen(ctx, input->isFullscreen);
        }

        gui_layoutRowDynamic(ctx, 20, 1);
        if (gui_button(ctx, input->paused ? "Unpause" : "Pause")) {
            input->paused = !input->paused;
        }

        gui_treePop(ctx);
    }
}

/**
 * Renders main menu
 */
static void gui_renderMenu(ProgContext ctx, InputData *input) {
    if (!input->showMenu) {
        return;
    }

    int h;
    window_getRealSize(ctx, NULL, &h);
    float height = 0.95f * h;

    if (gui_beginTitled(ctx, GUI_WINDOW_MENU, "Settings",
        nk_rect(15, 15, 250, height),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
        NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        renderGeneral(ctx, input);
        renderPhysics(ctx, input);
        renderSettings(ctx, input);
    }
    gui_end(ctx);
}

////////////////////////    PUBLIC    ////////////////////////////

void gui_renderContent(ProgContext ctx) {
    InputData *input = getInputData();

    gui_renderHelp(ctx, input);
    gui_renderMenu(ctx, input);
}