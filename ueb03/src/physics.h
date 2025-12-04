/**
* @file physics.h
 * @brief Physics simulation with black holes and goal detection
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

void physics_drawBlackHoles(void);

void physics_drawGoal(void);

void physics_addBall(void);

void physics_removeBall(void);

void physics_addBlackHole(void);

void physics_removeBlackHole(void);

void physics_orderBallsDiagonally(void);

void physics_orderBallsRandom(void);

void physics_orderBallsAroundMax(void);

bool physics_isGameWon(void);

bool physics_isGameLost(void);

void physics_resetGame(void);

int physics_getBallCount(void);

int physics_getBlackHoleCount(void);

void physics_kickBall(void);

#endif // PHYSICS_H
