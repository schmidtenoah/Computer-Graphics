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
#define MAX_STARS 40

/**
 * Function pointer type for curve evaluation functions (utils_evalSpline / utils_evalBezier)
 * Used to switch between different curve algorithms (spline/bezier) at runtime.
 *
 * @param ctrl Array of control points
 * @param numPoints Number of control points
 * @param t Parameter value [0,1] along the curve
 * @param dest Output: calculated point on curve at parameter t
 * @param updateCoeffs Pointer to flag indicating if coefficients need recalculation
 */
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
    bool paused;
    struct {
        int button;
        int action;
        float xPos, yPos;
    } mouse;

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
        bool collected[MAX_STARS];
        int currentLevel;
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