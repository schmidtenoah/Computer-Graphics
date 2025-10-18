#include "utils.h"
#include "rendering.h"
#include "input.h"

// LOCAL

static const float uniformBSplineMatrix[4][4] = {
        { -1,  3, -3,  1 },
        {  3, -6,  3,  0 },
        { -3,  0,  3,  0 },
        {  1,  4,  1,  0 }
};

static const float bezierMatrix[4][4] = {
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

// PUBLIC

void utils_bSplineUniform(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t, vec2 dest) {
    float T[4] = { t*t*t, t*t, t, 1 };

    float Gx[4] = { p0[0], p1[0], p2[0], p3[0] };
    float Gy[4] = { p0[1], p1[1], p2[1], p3[1] };

    // Compute T * M
    float TM[4];
    for (int j = 0; j < 4; ++j) {
        TM[j] = (T[0] * uniformBSplineMatrix[0][j] +
                 T[1] * uniformBSplineMatrix[1][j] +
                 T[2] * uniformBSplineMatrix[2][j] +
                 T[3] * uniformBSplineMatrix[3][j]) / 6.0f;
    }

    // Multiply by control points G
    float x = TM[0]*Gx[0] + TM[1]*Gx[1] + TM[2]*Gx[2] + TM[3]*Gx[3];
    float y = TM[0]*Gy[0] + TM[1]*Gy[1] + TM[2]*Gy[2] + TM[3]*Gy[3];

    dest[0] = x;
    dest[1] = y;
}

void utils_bSplineUniformGlobal(vec2* ctrl, int numPoints, float T, vec2 dest) {
    if (numPoints < 4) {
        return;
    }

    int numSegments = numPoints - 3;
    if (numSegments <= 0) {
        dest[0] = 0;
        dest[1] = 0;
        return;
    }

    // Clamp T to [0, 1]
    if (T < 0.0f) T = 0.0f;
    if (T > 1.0f) T = 1.0f;

    // Compute segment index and local t
    float segmentPos = T * numSegments;
    int i = (int)floorf(segmentPos);
    if (i >= numSegments) i = numSegments - 1; // clamp to last
    float t = segmentPos - i; // fractional part

    utils_bSplineUniform(ctrl[i], ctrl[i+1], ctrl[i+2], ctrl[i+3], t, dest);
}

void utils_bezier(vec2* ctrl, int numPoints, float t, vec2 dest) {
    if (numPoints != 4) {
        return;
    }

    float T[4] = { t*t*t, t*t, t, 1 };

    float Gx[4] = { ctrl[0][0], ctrl[1][0], ctrl[2][0], ctrl[3][0] };
    float Gy[4] = { ctrl[0][1], ctrl[1][1], ctrl[2][1], ctrl[3][1] };

    // Compute T * M
    float TM[4];
    for (int j = 0; j < 4; ++j) {
        TM[j] = (T[0] * bezierMatrix[0][j] +
                 T[1] * bezierMatrix[1][j] +
                 T[2] * bezierMatrix[2][j] +
                 T[3] * bezierMatrix[3][j]);
    }

    // Multiply by control points G
    float x = TM[0]*Gx[0] + TM[1]*Gx[1] + TM[2]*Gx[2] + TM[3]*Gx[3];
    float y = TM[0]*Gy[0] + TM[1]*Gy[1] + TM[2]*Gy[2] + TM[3]*Gy[3];

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

    curveFn(ctrl, n, t, p1);
    curveFn(ctrl, n, t + eps, p2);

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
