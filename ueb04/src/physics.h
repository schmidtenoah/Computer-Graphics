/**
* @file physics.h
 * @brief Physics simulation with Euler integration
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <fhwcg/fhwcg.h>

/**
 * Initializes physics system with two spheres
 */
void physics_init(void);

/**
 * Updates physics simulation (Euler integration)
 */
void physics_update(void);

/**
 * Cleans up physics resources
 */
void physics_cleanup(void);

/**
 * Renders all spheres
 */
void physics_drawSpheres(void);

/**
 * Toggles sphere wandering behavior
 */
void physics_toggleWander(void);

/**
 * Updates the number of particles in the simulation
 */
void physics_updateParticleCount(int count);

/**
 * Renders all particles
 */
void physics_drawParticles(void);

/**
 * Sets a new random leader particle
 */
void physics_setNewLeader(void);

/**
 * Moves the manual center position (for TM_CENTER mode)
 * @param delta Movement vector
 */
void physics_moveCenterManual(vec3 delta);

#endif // PHYSICS_H