/**
 * @file logic.h
 * @brief Game logic and level management
 *
 * Functions for program state, including patch and camera logic.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef LOGIC_H
#define LOGIC_H

#include <fhwcg/fhwcg.h>
#include "input.h"

typedef struct {
    mat4 coeffsY;
    int degS, degT;
} Patch;

typedef struct {
    float value;  // q(s,t)
    float dsd;    // ∂q/∂s
    float dtd;    // ∂q/∂t
} PatchEvalResult;

/**
 * Updates game logic per frame - called in rendering_draw().
 * Updates airplane position and rotation along the curve
 * Checks for star collection.
 * Detects cloud collisions.
 *
 * @param data Pointer to InputData containing game state
 */
void logic_update(InputData *data);

/**
 * Initializes logic system.
 * Sets up patch array for surface representation.
 */
void logic_init(void);

/**
 * Cleans up logic system resources.
 * Frees patch array and control points.
 */
void logic_cleanup(void);

/**
 * Debug function to print polynomial equations for all patches.
 * Outputs equations in the form: q(s,t) = c₀₀ + c₀₁*s + c₀₂*s² + ...
 */
void logic_printPolynomials(void);

/**
 * Initializes camera flight path from highest to lowest point on surface.
 * Creates a cubic Bezier curve with 4 control points:
 * - P0: highest point on surface
 * - P1: 1/3 along path, height follows surface + offset
 * - P2: 2/3 along path, height follows surface + offset
 * - P3: lowest point on surface
 *
 * @param data Application input data
 */
void logic_initCameraFlight(InputData *data);

/**
 * Updates camera position and direction during flight animation.
 * Evaluates position along Bezier curve and computes tangent for direction.
 * Flight completes when t reaches 1.0.
 *
 * @param data Application input data
 * @param deltaTime Time elapsed since last frame (seconds)
 */
void logic_updateCameraFlight(InputData *data, float deltaTime);

void logic_evalSplineGlobal(float gT, float gS, vec3 posDest, vec3 normalDest);

void logic_closestSplinePointTo(vec3 worldPos, float *outS, float *outT);

#endif // LOGIC_H
