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

/**
 * Target mode for particles - determines what particles move toward.
 */
typedef enum {
    TM_SPHERES,
    TM_CENTER, 
    TM_LEADER,
    TM_BOX_CENTER
} TargetMode;

/**
 * Visual representation mode for particles.
 */
typedef enum {
    SV_SPHERE, 
    SV_LINE, 
    SV_TRIANGLE
} SphereVis;

/**
 * Camera mode - either free or following lead particle.
 */
typedef enum {
    CAM_FREE,
    CAM_PARTICLE
} CameraMode;

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
        CameraMode mode;

        // Particle camera data
        vec3 particlePos;
        vec3 particleDir;
        vec3 particleUp;
        float behindDistance;
        float aboveDistance; 
    } cam;

    struct {
        bool texOrder1;
        float roomSize;
        bool dropShadows;
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