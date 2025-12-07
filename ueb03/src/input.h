/**
 * @file input.h
 * @brief Manage input and application state.
 *
 * Defines InputData struct containing all application states including
 * - user input (mouse, keyboard),
 * - curve settings,
 * - game state
 *
 * Provides functions to initialize input handling and register event callbacks.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef INPUT_H
#define INPUT_H

#include <fhwcg/fhwcg.h>

#define OBSTACLE_COUNT 6
#define OBSTACLE_HEIGHT 0.2f

/**
 * Dynamic array struct for vec3 elements.
 * Stores ctrl points
 */
typedef struct {
    size_t size;
    size_t capacity;
    vec3 *data;
} Vec3Arr;

/**
 * Struct for box obstacle
 * Position with surface coordinates (gS, gT).
 */
typedef struct {
    vec3 normal;
    vec3 center;
    float width, height, length;
    float gS, gT;
} Obstacle;

/**
 * Collision parameters for different collisions.
 */
typedef struct {
    float damping;
    float spring;
    bool enabled;
} Collision;

/** Struct containing all data for application state. */
typedef struct {
    bool isFullscreen;
    bool showWireframe;
    bool showHelp;
    bool showMenu;
    float deltaTime;
    bool showNormals;
    bool paused;

    struct {
        Camera *data;
        vec3 pos, dir;
        bool isFlying;
        struct {
            vec3 p0, p1, p2, p3;
            float t;
            float duration;
            bool showPath;
        } flight;
    } cam;

    struct {
        int dimension;
        int resolution;
        float controlPointOffset;
        bool resolutionChanged;
        bool dimensionChanged;
        bool offsetChanged;
        bool showControlPoints;
        bool showSurface;
        Vec3Arr controlPoints;
        bool useTexture;
        int currentTextureIndex;
        float textureTiling;  // Texture repeat factor
        vec3 minPoint;
        vec3 maxPoint;
        bool extremesValid;
    } surface;

    struct {
        float selectedYChange;
        int selectedCp;
        int skipCnt;
        bool pressingUp, pressingDown;
    } selection;

    struct {
        vec3 posWS;
        vec3 color;
        vec3 falloff;  // x: constant, y: linear, z: quadratic
        bool enabled;
        float ambientFactor;
        bool visualize;
        vec3 center;
        float currAngle;
        float rotationRadius;
        float speed;
    } pointLight;

    struct {
        float gravity;
        float fixedDt;
        float dtAccumulator;
        float mass;
        float ballRadius;
        float frictionFactor;
        float ballSpawnRadius;
        float blackHoleStrength;
        float blackHoleRadius;
        float blackHoleCaptureRadius;
        float kickStrength;

        Collision ball;
        Collision wall;
        Collision obs;
    } physics;

    struct {
        Obstacle obstacles[OBSTACLE_COUNT];
        int selectedIdx;
        int obstacleCnt;
        bool showObstacles;
        bool paused;
    } game;

} InputData;

/**
 * Initializes all default values for input struct
 *
 * @param ctx Program context
 */
void input_init(ProgContext ctx);

/**
 * Returns pointer to InputData struct
 * Allows other modules to access and modify values
 *
 * @return Pointer to InputData
 */
InputData* getInputData(void);

/**
 * Register all callback functions (w/ GLFW)
 *
 * @param ctx Program Context
 */
void input_registerCallbacks(ProgContext ctx);

#endif // INPUT_H