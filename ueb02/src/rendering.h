/**
 * @file rendering.h
 * @brief Rendering for curve visualization and interactive game objects.
 *
 * Key functions:
 * - Curve evaluation and visualization (spline/bezier)
 * - Interactive control point buttons with mouse dragging
 * - Game object rendering (airplane, stars, clouds)
 * - Debug overlays (wireframe, collision circles, normals, convex hull)
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef RENDERING_H
#define RENDERING_H

#include <fhwcg/fhwcg.h>

//PUBLIC

/**
 * Initializes rendering.
 * Loads shaders, sets up viewport and
 * creates initial control point buttons.
 */
void rendering_init(void);

/**
 * Renders the entire scene (every frame).
 * Draws background, curve, control points, airplane, stars, clouds,
 * and optional overlays (wireframe, colliders, normals, convex hull).
 */
void rendering_draw(void);

/**
 * Cleans up all rendering resources.
 * Frees memory.
 */
void rendering_cleanup(void);

/**
 * Handles window resize events.
 * Updates viewport, recalculates projection matrix.
 *
 * @param width New window width
 * @param height New window height
 */
void rendering_resize(int width, int height);

#endif // RENDERING_H