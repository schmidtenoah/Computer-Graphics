/**
 * @file utils.c
 * @brief Implementation of mathematical utility functions for surface and curve operations.
 *
 * Implements B-spline surface evaluation
 *
 * Additional utilities:
 * - Height functions for surface initialization
 * - Cubic Bezier curve evaluation for camera path
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "utils.h"
#include "rendering.h"
#include "input.h"
#include "logic.h"

////////////////////////    LOCAL    ////////////////////////////

typedef void (*HeightFunc)(vec3 *cp, int x, int z, int dimension);

/**
 * B-spline basis matrix (transposed).
 * Used for B-spline patches
 */
static mat4 splineMatrixTransposed = {
        { -1,  3, -3,  1 },
        {  3, -6,  3,  0 },
        { -3,  0,  3,  0 },
        {  1,  4,  1,  0 }
};

/**
 * B-spline basis matrix.
 * Defines the B-spline blending functions
 */
static mat4 splineMatrix = {
        { -1,  3, -3,  1 },
        {  3, -6,  0,  4 },
        { -3,  3,  3,  1 },
        {  1,  0,  0,  0 }
};

/**
 * Height function: Flat plane at y=0.
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_flat(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(x);
    NK_UNUSED(z);

    (*cp)[1] = 0.0f;
}

/**
 * Height function: Sinus pattern.
 * Creates a wave surface using sin(x)·cos(z).
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_sin(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);

    float freq = 0.5f;
    (*cp)[1] = sinf(x * freq) * cosf(z * freq) * 2.0f;
}

/**
 * Height function: Cosine pattern.
 * Creates rolling hills using cos(x) + sin(z).
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_cos(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);

    float freq = 0.4f;
    (*cp)[1] = cosf(x * freq) + sinf(z * freq);
}

/**
 * Height function: Gauss bell curve.
 * Creates a smooth peak at the center of the surface.
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_gauss(vec3 *cp, int x, int z, int dimension) {
    float cx = (dimension - 1) / 2.0f;
    float cz = (dimension - 1) / 2.0f;
    float sigma = dimension / 4.0f;

    float dx = x - cx;
    float dz = z - cz;
    float dist2 = dx * dx + dz * dz;

    (*cp)[1] = expf(-dist2 / (2.0f * sigma * sigma)) * 5.0f;
}

/**
 * Height function: Random noise.
 * Generates random heights for a rough, irregular surface.
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_random(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(x);
    NK_UNUSED(z);

    float r = ((float)rand() / RAND_MAX) * 5.0f - 2.5f;
    (*cp)[1] = r;
}

/**
 * Height function: hill.
 * Creates a smooth hill that rises from edges to center
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_hill(vec3 *cp, int x, int z, int dimension) {
    float cx = (dimension - 1) / 2.0f;
    float cz = (dimension - 1) / 2.0f;

    float dx = (x - cx) / cx;
    float dz = (z - cz) / cz;
    float dist = sqrtf(dx * dx + dz * dz);

    float height = cosf(dist * (float)M_PI / 2.0f);
    if (height < 0.0f) height = 0.0f;
    (*cp)[1] = height * 5.0f;
}

/**
 * Height function: Exponential.
 * Creates a sharp peak at origin with exponential falloff.
 *
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_exp(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    float exp = expf(-(x * x + z * z) / 100.0f);
    (*cp)[1] = exp * 10.0f;
}

/**
 * TODO
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_tiltX(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(z);

    (*cp)[1] -= 0.02f * x;
}

/**
 * TODO
 * @param cp Pointer to control point to modify
 * @param x X grid index
 * @param z Z grid index
 * @param dimension Grid dimension
 */
static void height_tiltZ(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(x);

    (*cp)[1] -= 0.02f * z;
}


/**
 * Array of height function pointers, indexed by HeightFuncType enum.
 */
static HeightFunc g_heightFuncs[HF_COUNT] = {
    height_flat,   // HF_FLAT
    height_sin,    // HF_SIN
    height_cos,    // HF_COS
    height_gauss,  // HF_GAUSS
    height_random, // HF_RANDOM
    height_hill,   // HF_HILL
    height_exp,    // HF_EXP
    height_tiltX,  // HF_TILT_X
    height_tiltZ   // HF_TILT_Z
};

////////////////////////    PUBLIC    ////////////////////////////

void utils_applyHeightFunction(HeightFuncType funcType) {
    if (funcType < 0 || funcType >= HF_COUNT) {
        return;
    }

    InputData *data = getInputData();
    int dimension = data->surface.dimension;

    for (int z = 0; z < dimension; ++z) {
        for (int x = 0; x < dimension; ++x) {
            int idx = z * dimension + x;
            g_heightFuncs[funcType](&data->surface.controlPoints.data[idx], x, z, dimension);
        }
    }

    data->surface.dimensionChanged = true;
    data->surface.resolutionChanged = true;
    data->surface.offsetChanged = true;
}

void utils_calculatePolynomialPatch(Patch *p, mat4 geometryTerm) {
    mat4 tmp, result;

    // tmp = G * M^T
    glm_mat4_mul(geometryTerm, splineMatrixTransposed, tmp);

    // result = M * (tmp)
    glm_mat4_mul(splineMatrix, tmp, result);

    const float scale = 36.0f;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            p->coeffsY[col][row] = result[col][row] / scale;
        }
    }
}

PatchEvalResult utils_evalPatchLocal(Patch *p, float s, float t) {
    PatchEvalResult r = {0};

    // Basis vectors
    vec4 sVec  = { s*s*s, s*s, s, 1.0f };
    vec4 tVec  = { t*t*t, t*t, t, 1.0f };
    vec4 dsVec = { 3*s*s, 2*s, 1.0f, 0.0f };
    vec4 dtVec = { 3*t*t, 2*t, 1.0f, 0.0f };

    vec4 temp, temp_dt;

    // Compute intermediate vectors
    glm_mat4_mulv(p->coeffsY, tVec, temp);     // C * tVec
    glm_mat4_mulv(p->coeffsY, dtVec, temp_dt); // C * dtVec

    // Evaluate
    r.value = glm_vec4_dot(sVec, temp);
    r.dsd   = glm_vec4_dot(dsVec, temp);
    r.dtd   = glm_vec4_dot(sVec, temp_dt);

    assert(!isnan(r.value));
    return r;
}

void utils_evalBezier3D(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t, vec3 out) {
    // Cubic Bezier: B(t) = (1-t)³p0 + 3(1-t)²t*p1 + 3(1-t)t²p2 + t³p3
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;

    float b0 = mt3;
    float b1 = 3.0f * mt2 * t;
    float b2 = 3.0f * mt * t2;
    float b3 = t3;

    out[0] = b0 * p0[0] + b1 * p1[0] + b2 * p2[0] + b3 * p3[0];
    out[1] = b0 * p0[1] + b1 * p1[1] + b2 * p2[1] + b3 * p3[1];
    out[2] = b0 * p0[2] + b1 * p1[2] + b2 * p2[2] + b3 * p3[2];
}

void utils_evalBezierTangent3D(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t, vec3 out) {
    // Derivative of cubic Bezier: B'(t) = 3(1-t)²(p1-p0) + 6(1-t)t(p2-p1) + 3t²(p3-p2)
    float t2 = t * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;

    float b0 = 3.0f * mt2;
    float b1 = 6.0f * mt * t;
    float b2 = 3.0f * t2;

    vec3 d01, d12, d23;
    glm_vec3_sub(p1, p0, d01);
    glm_vec3_sub(p2, p1, d12);
    glm_vec3_sub(p3, p2, d23);

    out[0] = b0 * d01[0] + b1 * d12[0] + b2 * d23[0];
    out[1] = b0 * d01[1] + b1 * d12[1] + b2 * d23[1];
    out[2] = b0 * d01[2] + b1 * d12[2] + b2 * d23[2];

    glm_vec3_normalize(out);
}

void utils_rotateAroundYAxis(vec3 *currPos, float *currAngle, vec3 center, float radius, float speed, float deltaTime) {
    *currAngle += speed * deltaTime;

    // Wrap angle to avoid overflow
    if (*currAngle > 2.0f * M_PI) *currAngle -= 2.0f * (float) M_PI;

    // Compute new position around Y-axis
    (*currPos)[0] = center[0] + radius * cosf(*currAngle);
    (*currPos)[1] = center[1];
    (*currPos)[2] = center[2] + radius * sinf(*currAngle);
}

void utils_getNormal(float dsd, float dtd, float stepX, float stepZ, vec3 dest){
    vec3 n = { 0 };
    vec3 rs = { 0.0f, dsd, stepZ };
    vec3 rt = { stepX, dtd, 0.0f };
    glm_vec3_cross(rs, rt, n);
    glm_vec3_normalize_to(n, dest);
}

void utils_closestPointOnAABB(vec3 point, Obstacle *o, vec3 dest) {
    vec3 relativePoint;
    glm_vec3_sub(point, o->center, relativePoint);

    float ex = o->length;
    float ey = o->height;
    float ez = o->width;

    // clamp inside box
    relativePoint[0] = glm_clamp(relativePoint[0], -ex, ex);
    relativePoint[1] = glm_clamp(relativePoint[1], -ey, ey);
    relativePoint[2] = glm_clamp(relativePoint[2], -ez, ez);

    glm_vec3_add(o->center, relativePoint, dest);
}

void utils_getAABBNormal(Obstacle *o, vec3 pos, float dist, vec3 diff, vec3 dest) {
    vec3 normal;
    if (dist < 1e-6f) {
        float ex = o->length;
        float ey = o->height;
        float ez = o->width;

        float dx = ex - fabsf(pos[0] - o->center[0]);
        float dy = ey - fabsf(pos[1] - o->center[1]);
        float dz = ez - fabsf(pos[2] - o->center[2]);

        // smallest distance to box -> direction o biggest penetration
        if (dx <= dy && dx <= dz) {
            normal[0] = (pos[0] > o->center[0]) ? 1.0f : -1.0f;
            normal[1] = 0.0f;
            normal[2] = 0.0f;
        } else if (dy <= dz) {
            normal[0] = 0.0f;
            normal[1] = (pos[1] > o->center[1]) ? 1.0f : -1.0f;
            normal[2] = 0.0f;
        } else {
            normal[0] = 0.0f;
            normal[1] = 0.0f;
            normal[2] = (pos[2] > o->center[2]) ? 1.0f : -1.0f;
        }
    } else {
        glm_vec3_scale(diff, 1.0f / dist, normal);
    }

    glm_vec3_copy(normal, dest);
}
