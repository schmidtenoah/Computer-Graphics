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

typedef struct {
    size_t size;
    size_t capacity;
    vec3 *data;
} Vec3Arr;

// TODO: REMOVE
typedef void (*CurveEvalFn)(vec2* ctrl, int numPoints, float t, vec2 dest, bool *updateCoeffs);

/** Struct containing all data for application state
* - GUI
* - Curve
* - Airplane
* - Game
*/
typedef struct {
    bool isFullscreen;
    bool showWireframe;
    bool showHelp;
    bool showMenu;
    float deltaTime;
    bool showNormals;
    bool paused;

    // seperate pos and dir because we need to manually change the camera
    struct {
        Camera *data;
        vec3 pos, dir;
        bool isFlying;
    } cam;

    struct {
        int button;
        int action;
        float xPos, yPos;
    } mouse;

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
    } surface;

    struct {
        float selectedYChange;
        int selectedCp;
        int skipCnt;
        bool pressingUp, pressingDown;
    } selection;

    struct {
        CurveEvalFn curveEval;
        float width;
        float resolution;
        bool drawPolygon;
        bool drawConvexHull;
        bool showNormals;
        int buttonCount;
        bool resolutionChanged;
        bool buttonsChanged;
    } curve;

    struct {
        bool isFlying;
        bool showColliders;
        struct {
            vec2 position;
            float rotation;
            vec2 vertices[3];
            float colliderRadius;
            float defaultSpeed;
        } airplane;
        struct {
            vec2 *pos;
            int n;
            float colliderRadius;
        } stars, clouds;
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