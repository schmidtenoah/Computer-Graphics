/**
 * @file physics.h
 * @brief TODO
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include <fhwcg/fhwcg.h>

void physics_init(void);

void physics_update(void);

void physics_cleanup(void);

void physics_drawBalls(void);
void physics_addBall(void);
void physics_removeBall(void);

#endif // PHYSICS_H
