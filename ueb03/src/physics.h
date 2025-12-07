/**
* @file physics.h
 * @brief Physics simulation with black holes and goal detection
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <fhwcg/fhwcg.h>

/**
 * Initializes the physics system
 */
void physics_init(void);

/**
 * Updates physics simulation for one frame.
 */
void physics_update(void);

/**
 * Cleans up all physics resources.
 * Frees ball, black hole arrays, wall data.
 */
void physics_cleanup(void);

/**
 * Renders all active balls as spheres.
 * Uses ball radius from physics parameters.
 * Inactive balls (captured by black holes) are not drawn.
 */
void physics_drawBalls(void);

/**
 * Renders all black holes as semi-transparent dark spheres.
 * Uses black hole radius from physics params.
 */
void physics_drawBlackHoles(void);

/**
 * Renders the goal as a semi-transparent green sphere.
 * Goal is positioned at the lowest point on the surface.
 */
void physics_drawGoal(void);

/**
 * Adds a new ball to the simulation at a random surface position.
 * Ball is initialized with zero velocity and placed on surface.
 */
void physics_addBall(void);

/**
 * Removes the last ball from the simulation.
 * Does nothing if ball array is empty.
 */
void physics_removeBall(void);

/**
 * Adds a new black hole at a random surface position.
 */
void physics_addBlackHole(void);

/**
 * Removes the last black hole from the simulation.
 * Does nothing if black hole array is empty.
 */
void physics_removeBlackHole(void);

/**
 * Resets ball positions to diagonal line across surface.
 * Balls are distributed evenly along diagonal from (0,0) to (1,1).
 * Velocities are reset to zero.
 */
void physics_orderBallsDiagonally(void);

/**
 * Resets ball positions to random locations on surface.
 * Velocities are reset to zero.
 */
void physics_orderBallsRandom(void);

/**
 * Resets ball positions in a circle around the highest surface point.
 */
void physics_orderBallsAroundMax(void);

/**
 * Checks if the game win condition is met.
 * Win condition: at least one ball reached the goal.
 *
 * @return true if goal was reached, false otherwise
 */
bool physics_isGameWon(void);

/**
 * Checks if the game lose condition is met.
 * Lose condition: all balls captured by black holes before reaching goal.
 *
 * @return true if all balls inactive and goal not reached, false otherwise
 */
bool physics_isGameLost(void);

/**
 * Resets the entire game state.
 * Calls physics_init() to reset all game elements.
 */
void physics_resetGame(void);

/**
 * Counts the number of active balls in the simulation.
 * Inactive balls (captured by black holes) are not counted.
 *
 * @return Number of active balls
 */
int physics_getBallCount(void);

/**
 * Gets the total number of black holes in the simulation.
 *
 * @return Number of black holes
 */
int physics_getBlackHoleCount(void);

/**
 * Applies a random impulse to the first active ball.
 */
void physics_kickBall(void);

#endif // PHYSICS_H
