/**
 * @file utils.h
 * @brief Utility functions for curve mathematics, hull and collision detection
 *
 * Provides mathematical stuff for the curve visualization including:
 * - Curve evaluation (B-spline and Bezier curve)
 * - Convex hull
 * - Circle-circle collision detection
 * - Normal vector calculation for curve vertices
 * - Tangent vector computation for curve orientation
 * - Mouse-circle intersection testing for UI interaction
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef UTILS_H
#define UTILS_H

#include <fhwcg/fhwcg.h>
#include "rendering.h"
#include "input.h"

#define VEC2(x, y) ((vec2) {x, y})

/**
 * Computes convex hull with 2D points
 * -> smallest convex polygon surrounding all points
 *
 * @param points 2D vector of input points
 * @param hull Output 2D vector for convex hull vertices
 * @param n Number of input points
 * @return Number of vertices in the convex hull
 */
int utils_convexHullVec2(vec2* points, vec2* hull, int n);

/**
 * Evaluates Bezier curve
 *
 * @note Only works with exactly 4 control points.
 *
 * @param ctrl 2D vector of 4 control points defining the Bezier curve
 * @param numPoints Number of control points (must be 4)
 * @param T Parameter value in range [0, 1] where 0=start, 1=end
 * @param dest Output parameter - resulting 2D point on the curve at parameter T
 * @param updateCoeffs Pointer to flag indicating if coefficients need recalculation
 */
void utils_evalBezier(vec2 *ctrl, int numPoints, float T, vec2 dest, bool *updateCoeffs);

/**
 * Evaluates B-spline curve
 *
 * @param ctrl 2D vector of control points (minimum 4 required)
 * @param numPoints Number of control points
 * @param T Parameter value in range [0, 1] where 0=start, 1=end
 * @param dest Output parameter - resulting 2D point on the curve at parameter T
 * @param updateCoeffs Pointer to flag indicating if coefficients need recalculation
 */
void utils_evalSpline(vec2 *ctrl, int numPoints, float T, vec2 dest, bool *updateCoeffs);

/**
 * Calculates tangent at a point on a curve
 * The tangent indicates the direction of the curve at the given parameter value.
 *
 * @param curveFn Function pointer to curve evaluation function (spline or bezier)
 * @param ctrl 2D vector of control points
 * @param n Number of control points
 * @param t Parameter value in range [0, 1]
 * @param tangent Output parameter - normalized tangent vector at parameter t
 */
void utils_getTangent(CurveEvalFn curveFn, vec2 *ctrl, int n, float t, vec2 tangent);

/**
 * Tests if two circles overlap.
 *
 * @param c1 Center position of first circle
 * @param r1 Radius of first circle
 * @param c2 Center position of second circle
 * @param r2 Radius of second circle
 * @return true if circles overlap, false otherwise
 */
bool utils_circleInCircle(vec2 c1, float r1, vec2 c2, float r2);

/**
 * Calculates normal vectors for each vertex in a curve.
 * Normals are perpendicular to the curve tangent and point to the "outside".
 * Uses finite differences and cross product for calculation.
 *
 * @param vertices 2D vector of curve vertex positions
 * @param normalDest Output array for normal vectors
 * @param n Number of vertices
 */
void utils_calcNormals(vec2 *vertices, vec3 *normalDest, int n);

/**
 * Tests if the mouse cursor is currently inside the given circle.
 * @param mouseX Position of the mouse in x.
 * @param mouse Position of the mouse in y.
 * @param c Circle data.
 * @param rd Screen data.
 * @param range Range factor for the collision radius.
 * @returns If the mouse cursor is currently inside the given circle.
 */
bool utils_isMouseInCircle(float mouseX, float mouseY, Circle *c, RenderingData *rd, float range);

#endif // UTILS_H
