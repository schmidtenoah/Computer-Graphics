/**
* @file logic.h
 * @brief Minimal logic interface
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef LOGIC_H
#define LOGIC_H

#include <fhwcg/fhwcg.h>
#include "input.h"

/**
 * Updates game logic per frame
 * @param data Pointer to InputData containing game state
 */
void logic_update(InputData *data);

/**
 * Initializes logic system
 */
void logic_init(void);

/**
 * Cleans up logic system resources
 */
void logic_cleanup(void);

#endif // LOGIC_H