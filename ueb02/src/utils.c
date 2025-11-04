/**
 * @file utils.c
 * @brief Implementation of mathematical utility functions for curve rendering and collision detection.
 *
 * Implements curve evaluation using matrix-based approach:
 * - B-spline curves: Smooth curves passing near control points with C2 continuity
 * - Bezier curves: Curves with endpoints at first/last control points
 *
 * The general form is: P(t) = at³ + bt² + ct + d, where coefficients are computed
 * from control points using matrices.
 *
 * Additional utilities include:
 * - Convex hull calc
 * - Tangent calc
 * - Normal vector calc
 * - Collision detection
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "utils.h"
#include "rendering.h"
#include "input.h"
#include "logic.h"

////////////////////////    LOCAL    ////////////////////////////

typedef void (*HeightFunc)(vec3 *cp, int x, int z, int dimension);

static mat4 splineMatrixTransposed = {
        { -1,  3, -3,  1 },
        {  3, -6,  3,  0 },
        { -3,  0,  3,  0 },
        {  1,  4,  1,  0 }
};

static mat4 splineMatrix = {
        { -1,  3, -3,  1 },
        {  3, -6,  0,  4 },
        { -3,  3,  3,  1 },
        {  1,  0,  0,  0 }
};

static void height_flat(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(x);
    NK_UNUSED(z);

    (*cp)[1] = 0.0f;
}

static void height_sin(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);

    float freq = 0.5f;
    (*cp)[1] = sinf(x * freq) * cosf(z * freq) * 2.0f;
}

static void height_cos(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);

    float freq = 0.4f;
    (*cp)[1] = cosf(x * freq) + sinf(z * freq);
}

static void height_gauss(vec3 *cp, int x, int z, int dimension) {
    float cx = (dimension - 1) / 2.0f;
    float cz = (dimension - 1) / 2.0f;
    float sigma = dimension / 4.0f;

    float dx = x - cx;
    float dz = z - cz;
    float dist2 = dx * dx + dz * dz;

    (*cp)[1] = expf(-dist2 / (2.0f * sigma * sigma)) * 5.0f;
}

static void height_random(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    NK_UNUSED(x);
    NK_UNUSED(z);

    float r = ((float)rand() / RAND_MAX) * 5.0f - 2.5f;
    (*cp)[1] = r;
}

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

static void height_exp(vec3 *cp, int x, int z, int dimension) {
    NK_UNUSED(dimension);
    float exp = expf(-(x * x + z * z) / 100.0f);
    (*cp)[1] = exp * 10.0f;
}

static HeightFunc g_heightFuncs[HF_COUNT] = {
    height_flat,   // HF_FLAT
    height_sin,    // HF_SIN
    height_cos,    // HF_COS
    height_gauss,  // HF_GAUSS
    height_random, // HF_RANDOM
    height_hill,   // HF_HILL
    height_exp     // HF_EXP
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