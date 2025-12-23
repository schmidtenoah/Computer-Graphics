/**
* @file main.c
 * @brief Main entry point
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include <fhwcg/fhwcg.h>
#include "gui.h"
#include "input.h"
#include "rendering.h"
#include "model.h"
#include "physics.h"

#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 768

////////////////////////    LOCAL    ////////////////////////////

/**
 * Initializes all modules
 */
static void init(ProgContext ctx) {
    input_init(ctx);
    input_registerCallbacks(ctx);
    gui_init(ctx);
    model_init();
    physics_init();
    rendering_init();
    rendering_resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

/**
 * Cleans up all modules
 */
static void cleanup(ProgContext ctx) {
    gui_cleanup(ctx);
    model_cleanup();
    physics_cleanup();
    rendering_cleanup();
    window_cleanup(ctx);
}

////////////////////////    PUBLIC    ////////////////////////////

int main(void) {
    ProgContext ctx = window_init(
        PROGRAM_NAME,
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        1,
        HELP_SERVER_FLAGS | WINDOW_FLAGS_VSYNC
    );

    init(ctx);

    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);

    // Main rendering loop
    while (window_startNewFrame(ctx)) {
        InputData *d = getInputData();
        float dt = (float)window_getDeltaTime(ctx);
        d->deltaTime = d->paused ? 0.0f : dt;

        camera_updateCamera(d->cam.data, dt);
        physics_update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rendering_draw();
        gui_render(ctx, gui_renderContent);

        window_swapBuffers(ctx);
    }

    cleanup(ctx);
    return EXIT_SUCCESS;
}