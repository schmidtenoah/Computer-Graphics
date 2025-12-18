/**
* @file rendering.h
 * @brief Rendering system
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef RENDERING_H
#define RENDERING_H

#include <fhwcg/fhwcg.h>

/**
 * Initializes rendering system
 */
void rendering_init(void);

/**
 * Renders the entire scene
 */
void rendering_draw(void);

/**
 * Cleans up rendering resources
 */
void rendering_cleanup(void);

/**
 * Handles window resize
 * @param width New width
 * @param height New height
 */
void rendering_resize(int width, int height);

#endif // RENDERING_H