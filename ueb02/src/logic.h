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
 * Initializes the game logic.
 *
 * Sets airplane properties (collider radius, speed) and loads the first level.
 * Called once during program startup.
 */
void logic_init(void);

void logic_cleanup(void);

void logic_printPolynomials(void);

void logic_initCameraFlight(InputData *data);

void logic_updateCameraFlight(InputData *data, float deltaTime);

#endif // LOGIC_H