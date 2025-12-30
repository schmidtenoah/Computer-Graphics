/**
* @file input.h
 * @brief Input handling and application state management
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef INPUT_H
#define INPUT_H

#include <fhwcg/fhwcg.h>

#define START_NUM_PARTICLES 100

typedef enum {
    TM_SPHERES,
    TM_CENTER, 
    TM_LEADER,
    TM_BOX_CENTER
} TargetMode;

typedef enum {
    SV_SPHERE, 
    SV_LINE, 
    SV_TRIANGLE
} SphereVis;

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
        bool texOrder1;
        float roomSize;
    } rendering;

    struct {
        float fixedDt;
        float simulationSpeed;
        float dtAccumulator;

        float sphereRadius;
        float sphereSpeed;

        float roomForce;
    } physics;

    struct {
        int count;
        float gaussianConst;
        SphereVis sphereVis;
        TargetMode targetMode;
        float leaderKv;
        int leaderIdx;
        bool visVectors;
    } particles;

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