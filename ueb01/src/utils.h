#ifndef UTILS_H
#define UTILS_H

#include <fhwcg/fhwcg.h>
#include "rendering.h"
#include "input.h"

#define VEC2(x, y) ((vec2) {x, y})

int utils_convexHullVec2(vec2* points, vec2* hull, int n);

void utils_bezier(vec2* ctrl, int numPoints, float t, vec2 dest);

void utils_bSplineUniform(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t, vec2 dest);

void utils_bSplineUniformGlobal(vec2* ctrl, int numPoints, float T, vec2 dest);

void utils_getTangent(CurveEvalFn curveFn, vec2 *ctrl, int n, float t, vec2 tangent);

bool utils_circleInCircle(vec2 c1, float r1, vec2 c2, float r2);

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