/**
* @file input.h
 * @brief Input handling and application state management
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef INPUT_H
#define INPUT_H

#include <fhwcg/fhwcg.h>

/** Application state containing all settings and input data */
typedef struct {
    bool isFullscreen;
    bool showWireframe;
    bool showHelp;
    bool showMenu;
    bool paused;
    float deltaTime;

    struct {
        Camera *data;
        vec3 pos;
        vec3 dir;
    } cam;

    struct {
        int currentTextureIndex;  // 0 or 1 for walls/ceiling
    } rendering;

    struct {
        float gravity;
        float fixedDt;
        float dtAccumulator;
        float sphereRadius;
        float mass;
        float frictionFactor;
        float bounceDamping;
    } physics;

} InputData;

/**
 * Initializes input system and default values
 * @param ctx Program context
 */
void input_init(ProgContext ctx);

/**
 * Returns pointer to global input data
 * @return Pointer to InputData
 */
InputData* getInputData(void);

/**
 * Registers all input callbacks with GLFW
 * @param ctx Program context
 */
void input_registerCallbacks(ProgContext ctx);

#endif // INPUT_H