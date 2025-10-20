#include "utils.h"
#include "rendering.h"
#include "input.h"

#define MAX_SEGMENTS (BUTTON_COUNT - 3)

typedef struct {
    vec4 coeffsX;
    vec4 coeffsY;
} Segment;

// LOCAL

static Segment segments[MAX_SEGMENTS];

static const mat4 splineMatrix = {
        { -1,  3, -3,  1 },
        {  3, -6,  3,  0 },
        { -3,  0,  3,  0 },
        {  1,  4,  1,  0 }
};

static const mat4 bezierMatrix = {
        { -1,  3, -3,  1 },
        {  3, -6,  3,  0 },
        { -3,  3,  0,  0 },
        {  1,  0,  0,  0 }
};

static int findLowestPointVec2(vec2* points, int n) {
    int minIdx = 0;
    for (int i = 1; i < n; ++i) {
        if (points[i][1] < points[minIdx][1] ||
            (fabsf(points[i][1] - points[minIdx][1]) < EPSILON && points[i][0] < points[minIdx][0])) {
            minIdx = i;
        }
    }
    return minIdx;
}

static float angleBetweenVec2(vec2 from, vec2 to) {
    vec2 v;
    glm_vec2_sub(to, from, v);
    float angle = atan2f(v[1], v[0]);
    if (angle < 0) angle += 2.0f * (float) M_PI;
    return angle;
}

static void updateSplineCoefficients(vec2 *ctrl, int numPoints) {
    int numSegments = numPoints - 3;

    for (int i = 0; i < numSegments; ++i) {
        float Gx[4] = { ctrl[i][0], ctrl[i+1][0], ctrl[i+2][0], ctrl[i+3][0] };
        float Gy[4] = { ctrl[i][1], ctrl[i+1][1], ctrl[i+2][1], ctrl[i+3][1] };

        vec4 Cx = GLM_VEC4_ZERO_INIT;
        vec4 Cy = GLM_VEC4_ZERO_INIT;

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                Cx[row] += splineMatrix[row][col] * Gx[col];
                Cy[row] += splineMatrix[row][col] * Gy[col];
            }
            Cx[row] /= 6.0f;
            Cy[row] /= 6.0f;
        }

        glm_vec4_copy(Cx, segments[i].coeffsX);
        glm_vec4_copy(Cy, segments[i].coeffsY);
    }
}

static void updateBezierCoefficients(vec2 *ctrl, int numPoints) {
    int numSegments = numPoints - 3;

    for (int i = 0; i < numSegments; ++i) {
        float Gx[4] = { ctrl[i][0], ctrl[i+1][0], ctrl[i+2][0], ctrl[i+3][0] };
        float Gy[4] = { ctrl[i][1], ctrl[i+1][1], ctrl[i+2][1], ctrl[i+3][1] };

        float Cx[4] = {0}, Cy[4] = {0};

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                Cx[row] += bezierMatrix[row][col] * Gx[col];
                Cy[row] += bezierMatrix[row][col] * Gy[col];
            }
        }

        glm_vec4_copy(Cx, segments[i].coeffsX);
        glm_vec4_copy(Cy, segments[i].coeffsY);
    }
}

// PUBLIC

void utils_evalSpline(vec2 *ctrl, int numPoints, float T, vec2 dest, bool *updateCoeffs) {
    if (updateCoeffs != NULL && *updateCoeffs) {
        updateSplineCoefficients(ctrl, numPoints);
        *updateCoeffs = false;
    }

    int numSegments = numPoints - 3;
    T = glm_clamp(T, 0.0f, 1.0f);

    float segmentPos = T * numSegments;
    int i = (int) floorf(segmentPos);
    if (i >= numSegments) i = numSegments - 1;
    float t = segmentPos - i;

    Segment *s = &segments[i];

    // p(t)=((at+b)t+c)t+d == p(t)=at3+bt2+ct+d
    float x = ((s->coeffsX[0] * t + s->coeffsX[1]) * t + s->coeffsX[2]) * t + s->coeffsX[3];
    float y = ((s->coeffsY[0] * t + s->coeffsY[1]) * t + s->coeffsY[2]) * t + s->coeffsY[3];

    dest[0] = x;
    dest[1] = y;
}

void utils_evalBezier(vec2 *ctrl, int numPoints, float T, vec2 dest, bool *updateCoeffs) {
    if (numPoints != 4) {
        return;
    }

    if (updateCoeffs != NULL && *updateCoeffs) {
        updateBezierCoefficients(ctrl, numPoints);
        *updateCoeffs = false;
    }

    Segment *s = &segments[0];
    float t = glm_clamp(T, 0.0f, 1.0f);

    // p(t)=((at+b)t+c)t+d == p(t)=at3+bt2+ct+d
    float x = ((s->coeffsX[0] * t + s->coeffsX[1]) * t + s->coeffsX[2]) * t + s->coeffsX[3];
    float y = ((s->coeffsY[0] * t + s->coeffsY[1]) * t + s->coeffsY[2]) * t + s->coeffsY[3];

    dest[0] = x;
    dest[1] = y;
}

int utils_convexHullVec2(vec2* points, vec2* hull, int n) {
    if (n < 3) {
        return 0;
    }

    int hullCount = 0;

    int start = findLowestPointVec2(points, n);
    int current = start;
    float lastAngle = -1.0f; // previous angle in radians

    do {
        glm_vec2_copy(points[current], hull[hullCount++]);

        int nextPoint = -1;
        float minAngle = 2.0f * (float)M_PI + 1.0f; // larger than max angle

        for (int i = 0; i < n; ++i) {
            if (i == current) continue;

            float angle = angleBetweenVec2(points[current], points[i]);
            float relativeAngle = angle - lastAngle;
            if (relativeAngle <= 0) relativeAngle += 2.0f * (float)M_PI;

            if (relativeAngle < minAngle - EPSILON) {
                minAngle = relativeAngle;
                nextPoint = i;
            } else if (fabsf(relativeAngle - minAngle) < EPSILON) {
                // same angle, pick farther point
                float da = glm_vec2_distance2(points[current], points[nextPoint]);
                float db = glm_vec2_distance2(points[current], points[i]);
                if (db > da) nextPoint = i;
            }
        }

        lastAngle = angleBetweenVec2(points[current], points[nextPoint]);
        current = nextPoint;

    } while (current != start);

    glm_vec2_copy(hull[0], hull[hullCount++]);

    return hullCount;
}

void utils_getTangent(CurveEvalFn curveFn, vec2 *ctrl, int n, float t, vec2 tangent) {
    float eps = 0.001f; // small offset for numerical derivative
    vec2 p1, p2;

    curveFn(ctrl, n, t, p1, NULL);
    curveFn(ctrl, n, t + eps, p2, NULL);

    glm_vec2_sub(p2, p1, tangent);
    glm_vec2_normalize(tangent);
}

bool utils_circleInCircle(vec2 c1, float r1, vec2 c2, float r2) {
    float dx = c1[0] - c2[0];
    float dy = c1[1] - c2[1];
    float distSq = dx*dx + dy*dy;
    float radiusSum = r1 + r2;
    return distSq <= radiusSum * radiusSum;
}

bool utils_isMouseInCircle(float mouseX, float mouseY, Circle *c, RenderingData *rd, float range) {
    float mouseX_ndc = (float) mouseX / rd->screenRes[0];
    float mouseY_ndc = 1.0f - (float) mouseY / rd->screenRes[1];

    float sceneX = rd->left + mouseX_ndc * (rd->right - rd->left);
    float sceneY = rd->bottom + mouseY_ndc * (rd->top - rd->bottom);

    float dx = sceneX - c->center[0];
    float dy = sceneY - c->center[1];

    float radius = range * c->r;
    return (dx*dx + dy*dy) <= (radius * radius);
}

void utils_calcNormals(vec2 *vertices, vec3 *normalDest, int n) {
    for (int i = 0; i < n; ++i) {
    vec3 diff, normal;

        int left = (i == 0) ? 0 : i - 1;
        int right = (i == n - 1) ? n - 1 : i + 1;

        diff[0] = vertices[right][0] - vertices[left][0];
        diff[1] = vertices[right][1] - vertices[left][1];
        diff[2] = 0.0f;

        glm_vec3_cross(diff, (vec3) {0, 0, 1}, normal);
        glm_vec3_normalize(normal);

        glm_vec3_copy(normal, normalDest[i]);
    }
}
