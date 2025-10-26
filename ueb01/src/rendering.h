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

#define CURVE_MAX_VERTICES 10000

/** Small epsilon value for floating-point comparisons
 * to handle precision errors */
#define EPSILON 1e-6f

/** Maximum number of control point buttons (used by most complex level) */
#define BUTTON_COUNT 20

/**
 * Represents circle in 2D space.
 * Used for collision detection and button positioning.
 */
typedef struct {
    vec2 center;
    float r;
} Circle;

/**
 * Rendering viewport and projection data.
 * Contains screen resolution and projection bounds.
 */
typedef struct {
    ivec2 screenRes;
    float aspect;
    float left, right, top, bottom;
} RenderingData;

//PUBLIC

/**
 * Initializes rendering.
 * Loads shaders, sets up viewport and
 * creates initial control point buttons.
 */
void rendering_init(void);

/**
 * Initializes control point buttons for the current level.
 * Creates btnCnt draggable buttons.
 *
 * @param btnCnt Number of control point buttons of curve
 */
void initButtons(int btnCnt);

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
 * Updates viewport, recalculates projection matrix
 * and repositions buttons.
 *
 * @param width New window width
 * @param height New window height
 * @param btnCnt Number of control point buttons to reposition
 */
void rendering_resize(int width, int height, int btnCnt);

#endif // RENDERING_H