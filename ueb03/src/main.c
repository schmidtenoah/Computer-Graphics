/**
 * @file main.c
 * @brief Main entry point and game loop
 *
 * Initializes all subsystems (input, GUI, rendering, models, game logic) and runs
 * the main rendering loop. Manages program lifecycle from startup to shutdown.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include <fhwcg/fhwcg.h>
#include "gui.h"
#include "input.h"
#include "rendering.h"
#include "model.h"
#include "logic.h"

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 500

////////////////////////    LOCAL    ////////////////////////////

/**
 * Initializes all modules.
 * @param ctx The Program Context.
 */
static void init(ProgContext ctx) {
    input_init(ctx);
    input_registerCallbacks(ctx);
    logic_init();
    gui_init(ctx);
    model_init();
    rendering_init();
    rendering_resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

/**
 * Cleans all modules.
 * @param ctx The Program Context.
 */
static void cleanup(ProgContext ctx) {
    gui_cleanup(ctx);
    model_cleanup();
    rendering_cleanup();
    logic_cleanup();
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

    glClearColor(0.4f, 0.4f, 0.8f, 1.0f);

    // rendering loop
    while (window_startNewFrame(ctx)) {
        InputData *d = getInputData();
        float dt = (float) window_getDeltaTime(ctx);
        d->deltaTime = d->paused ? 0.0f : dt;
        camera_updateCamera(d->cam.data, dt);
        logic_update(d);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rendering_draw();
        gui_render(ctx, gui_renderContent);

        // switch front- and back-buffer
        window_swapBuffers(ctx);
    }

    cleanup(ctx);
    return EXIT_SUCCESS;
}
