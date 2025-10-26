/**
 * @file logic.h
 * @brief Game logic and level management
 *
 * Functions for game state, level progression and airplane "physics".
 * Handles collision detection, level loading and win/lose conditions.
 *
 * The game consists of 6 levels with varying difficulty:
 * - Collectible stars (must collect all to win)
 * - Cloud obstacles (collision causes level restart)
 * - Airplane follows the curve with slope-dependent speed
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef LOGIC_H
#define LOGIC_H

#include <fhwcg/fhwcg.h>
#include "input.h"

/**
 * Updates game logic per frame - called in rendering_draw().
 * Updates airplane position and rotation along the curve
 * Checks for star collection.
 * Detects cloud collisions.
 *
 * @param data Pointer to InputData containing game state
 * @param ctrl 2D vector of control points defining the curve
 * @param n Number of control points
 */
void logic_update(InputData *data, vec2 *ctrl, int n);

/**
 * Initializes the game logic.
 *
 * Sets airplane properties (collider radius, speed) and loads the first level.
 * Called once during program startup.
 */
void logic_init();

/**
 * Skips to the next level.
 * Stops airplane flight, resets curve parameter and loads the next level.
 * Wraps around to level 1 after level 6.
 *
 * @param data Pointer to InputData to update game state
 */
void logic_skipLevel(InputData *data);

/**
 * Restarts the current level.
 * Stops airplane flight, resets curve parameter and reloads level data.
 * All stars become "uncollected" again.
 *
 * @param data Pointer to InputData to update game state
 */
void logic_restartLevel(InputData *data);

/**
 * Loads a specific level by index.
 * Stops airplane flight, resets curve parameter and loads level configuration.
 * Used for level selection via num keys.
 *
 * @param idx Level index
 * @param data Pointer to InputData to update game state
 */
void loadLevel(int idx, InputData *data);

#endif // LOGIC_H