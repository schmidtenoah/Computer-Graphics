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

#endif // PHYSICS_H