/**
 * @file logic.c
 * @brief Implementation of game logic, physics and level management.
 *
 * Manages program state including surface generation, patch calculations,
 * and camera flight system.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "logic.h"
#include "input.h"
#include "model.h"
#include "utils.h"
#include "physics.h"

#include <fhwcg/fhwcg.h>

#define RANDOM_HEIGHT(scale) ((((float)rand() / RAND_MAX) - 0.5f) * scale)
#define CAMERA_HEIGHT_OFFSET 0.2f // Offset for 2nd and 3rd Ctrl.point of Bezier
#define LIGHT_OFFSET_Y 0.35f

DEFINE_ARRAY_TYPE(Patch, PatchArr)

////////////////////////    LOCAL    ////////////////////////////

/** Global array storing all polynomial patches for the surface */
static PatchArr g_patches;

/**
 * Updates control points when dimension or offset changes.
 * Preserves existing heights where possible and interpolates new points.
 *
 * @param cp Pointer to control points array to update
 * @param newDim New dimension (grid size) for control points
 * @param cpOffset Spacing offset between control points
 */
static void updateControlPoints(Vec3Arr *cp, int newDim, float cpOffset) {
    int oldDim = (int)sqrtf((float)cp->size);
    if (oldDim * oldDim != (int)cp->size) {
        oldDim = 0;
    }

    float newStep = (1.0f / (newDim - 1)) + cpOffset;
    Vec3Arr newPoints;
    vec3arr_init(&newPoints);
    vec3arr_reserve(&newPoints, newDim * newDim);

    for (int i = 0; i < newDim; ++i) {
        for (int j = 0; j < newDim; ++j) {
            vec3 p;
            p[0] = j * newStep;
            p[2] = i * newStep;

            float height = 0.0f;
            bool reused = false;

            // Reuse height from old grid if within bounds
            if (i < oldDim && j < oldDim && oldDim > 0) {
                height = cp->data[i * oldDim + j][1];
                reused = true;
            } else if (oldDim > 0) {
                // Interpolate from neighboring old points
                float sum = 0.0f;
                int count = 0;

                int ni = (int)roundf((float)i * (oldDim - 1) / (newDim - 1));
                int nj = (int)roundf((float)j * (oldDim - 1) / (newDim - 1));

                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        int oi = ni + di;
                        int oj = nj + dj;
                        if (oi >= 0 && oi < oldDim && oj >= 0 && oj < oldDim) {
                            sum += cp->data[oi * oldDim + oj][1];
                            count++;
                        }
                    }
                }

                if (count > 0)
                    height = sum / count;

                height += RANDOM_HEIGHT(0.2f);
            } else {
                height = RANDOM_HEIGHT(0.5f);
            }

            p[1] = height;
            vec3arr_push(&newPoints, p);
        }
    }

    vec3arr_free(cp);
    *cp = newPoints;
}

/**
 * Generates polynomial patches from control points using B-spline basis.
 * Creates (dimension-3)×(dimension-3) patches, each defined by a 4×4 grid
 * of control points.
 *
 * @param cp Pointer to control points array
 * @param dimension Grid dimension (number of control points per axis)
 */
static void updatePatchesFromControlPoints(Vec3Arr *cp, int dimension) {
    PatchArr_clear(&g_patches);
    Patch patch;

    for (int i = 0; i < dimension - 3; ++i) {
        for (int j = 0; j < dimension - 3; ++j) {
            mat4 geometryTerm = GLM_MAT4_ZERO_INIT;

            // Extract 4×4 height values for this patch
            for (int u = 0; u < 4; ++u) {
                for (int v = 0; v < 4; ++v) {
                    geometryTerm[v][u] = cp->data[(i + u) * dimension + (j + v)][1];
                }
            }

            utils_calculatePolynomialPatch(&patch, geometryTerm);
            PatchArr_push(&g_patches, patch);
        }
    }
}

/**
 * Generates complete surface mesh from patches.
 * Evaluates each patch at regular intervals to create a smooth surface.
 * Computes positions, normals, and texture coordinates for all vertices.
 *
 * @param cp Control points array
 * @param samples Number of samples per patch (grid resolution)
 * @param dimension Control point grid dimension
 * @param textureTiling Texture repeat factor
 * @param minPoint Output: lowest point on surface
 * @param maxPoint Output: highest point on surface
 * @param extremesValid Output: whether extreme points are valid
 * @param computeExtremes Whether to compute min/max points
 */
void generateSurfaceVertices(Vec3Arr *cp, int samples, int dimension, float textureTiling, vec3 minPoint, vec3 maxPoint,
    bool *extremesValid, bool computeExtremes) {
    int patchCount = dimension - 3;
    int gridSize   = (samples < 2) ? 2 : samples;
    int totalVerts = gridSize * gridSize;

    vec3 *positions = malloc(sizeof(vec3) * totalVerts);
    vec3 *normals   = malloc(sizeof(vec3) * totalVerts);
    vec2 *texcoords = malloc(sizeof(vec2) * totalVerts);

    float maxX = cp->data[dimension-1][0];
    float maxZ = cp->data[(dimension-1)*dimension][2];
    float stepX = maxX / (patchCount * 3.0f);
    float stepZ = maxZ / (patchCount * 3.0f);

    float minH =  1e10f;
    float maxH = -1e10f;
    vec3 locMin = {0.0f, 0.0f, 0.0f};
    vec3 locMax = {0.0f, 0.0f, 0.0f};

    // Sample surface at regular grid intervals
    for (int i = 0; i < gridSize; ++i) {
        float T_s = (float)i / (gridSize - 1); // global s
        float global_s = T_s * patchCount;
        int patch_s = (int)floor(global_s);
        if (patch_s >= patchCount) patch_s = patchCount - 1;
        if (patch_s < 0) patch_s = 0;
        float local_s = global_s - patch_s;

        for (int j = 0; j < gridSize; ++j) {
            float T_t = (float)j / (gridSize - 1); // global t
            float global_t = T_t * patchCount;
            int patch_t = (int)floor(global_t);
            if (patch_t >= patchCount) patch_t = patchCount - 1;
            if (patch_t < 0) patch_t = 0;
            float local_t = global_t - patch_t;

            Patch *p = &g_patches.data[patch_s * patchCount + patch_t];
            PatchEvalResult res = utils_evalPatchLocal(p, local_s, local_t);

            int idx = i * gridSize + j;

            // world position
            positions[idx][0] = (patch_t * 3 + local_t * 3) * stepX;
            positions[idx][2] = (patch_s * 3 + local_s * 3) * stepZ;
            positions[idx][1] = res.value;
            
            // normal from partial derivatives
            utils_getNormal(res.dsd, res.dtd, stepX, stepZ, normals[idx]);

            // TexCoords with tiling
            texcoords[idx][0] = T_s * textureTiling;
            texcoords[idx][1] = T_t * textureTiling;

            // extreme points based on interpolated surface
            if (computeExtremes) {
                float h = positions[idx][1];
                if (h > maxH) {
                    maxH = h;
                    glm_vec3_copy(positions[idx], locMax);
                }
                if (h < minH) {
                    minH = h;
                    glm_vec3_copy(positions[idx], locMin);
                }
            }
        }
    }

    if (computeExtremes) {
        glm_vec3_copy(locMin, minPoint);
        glm_vec3_copy(locMax, maxPoint);
        *extremesValid = true;
    }

    model_updateSurface(positions, normals, texcoords, gridSize);
    free(positions);
    free(normals);
    free(texcoords);
}

/**
 * Complete surface rebuild: updates control points, recalculates patches,
 * and generates new surface mesh.
 *
 * @param cp Control points array
 * @param dimension Control point grid dimension
 * @param samples Surface sampling resolution
 * @param cpOffset Control point spacing offset
 * @param textureTiling Texture repeat factor
 * @param minPoint Output: lowest point on surface
 * @param maxPoint Output: highest point on surface
 * @param extremesValid Output: whether extreme points are valid
 */
static void rebuildSurface(Vec3Arr *cp, int dimension, int samples, float cpOffset, float textureTiling, vec3 minPoint,
    vec3 maxPoint, bool *extremesValid) {
    updateControlPoints(cp, dimension, cpOffset);
    updatePatchesFromControlPoints(cp, dimension);
    generateSurfaceVertices(cp, samples, dimension, textureTiling, minPoint, maxPoint, extremesValid, true);
}

/**
 * Recalculates minimum and maximum points based on control point heights.
 * Used when control points are manually adjusted.
 *
 * @param cp Control points array
 * @param minPoint Output: lowest control point
 * @param maxPoint Output: highest control point
 * @param extremesValid Output: set to true if calculation succeeded
 */
static void recalculateExtremes(Vec3Arr *cp, vec3 minPoint, vec3 maxPoint, bool *extremesValid) {
    if (cp->size == 0) {
        *extremesValid = false;
        return;
    }

    float minHeight = 1e10f;
    float maxHeight = -1e10f;
    vec3 minPos, maxPos;

    for (size_t i = 0; i < cp->size; ++i) {
        float height = cp->data[i][1];
        
        if (height > maxHeight) {
            maxHeight = height;
            glm_vec3_copy(cp->data[i], maxPos);
        }
        if (height < minHeight) {
            minHeight = height;
            glm_vec3_copy(cp->data[i], minPos);
        }
    }

    glm_vec3_copy(minPos, minPoint);
    glm_vec3_copy(maxPos, maxPoint);
    *extremesValid = true;
}

/**
 * Checks for control point selection input and updates heights accordingly.
 * Handles up/down arrow key presses to adjust selected control point height.
 *
 * @param data input data
 */
static void checkSelectionState(InputData *data) {
    bool heightChanged = false;

    if (data->selection.pressingUp) {
        data->surface.controlPoints.data[data->selection.selectedCp][1] += data->selection.selectedYChange;
        data->surface.dimensionChanged = true;
        heightChanged = true;
    }

    if (data->selection.pressingDown) {
        data->surface.controlPoints.data[data->selection.selectedCp][1] -= data->selection.selectedYChange;
        data->surface.dimensionChanged = true;
        heightChanged = true;
    }

    // Recalculate extremes if height was manually changed
    if (heightChanged) {
        recalculateExtremes(
            &data->surface.controlPoints,
            data->surface.minPoint,
            data->surface.maxPoint,
            &data->surface.extremesValid
        );
    }
}

/**
 * Evaluates surface height at specific normalized coordinates.
 * Converts global (T_s, T_t) coordinates to local patch coordinates
 * and evaluates the corresponding polynomial.
 *
 * @param dimension Control point grid dimension
 * @param T_s Normalized s coordinate [0,1]
 * @param T_t Normalized t coordinate [0,1]
 * @return Height value at specified position
 */
static float evalSurfaceAt(int dimension, float T_s, float T_t) {
    int patchCount = dimension - 3;

    float global_s = T_s * patchCount;
    int patch_s = (int)floor(global_s);
    if (patch_s >= patchCount) patch_s = patchCount - 1;
    if (patch_s < 0) patch_s = 0;
    float local_s = global_s - patch_s;

    float global_t = T_t * patchCount;
    int patch_t = (int)floor(global_t);
    if (patch_t >= patchCount) patch_t = patchCount - 1;
    if (patch_t < 0) patch_t = 0;
    float local_t = global_t - patch_t;

    Patch *p = &g_patches.data[patch_s * patchCount + patch_t];
    PatchEvalResult res = utils_evalPatchLocal(p, local_s, local_t);

    return res.value;
}

static void updateObstacles(InputData *data) {
    for (int i = 0; i < OBSTACLE_COUNT; ++i) {
        Obstacle *o = &data->game.obstacles[i];
        logic_evalSplineGlobal(o->gT, o->gS, o->center, o->normal);
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void logic_update(InputData *data) {
    checkSelectionState(data);

    // Calculate all polynomials if geometry matrix changed
    if (data->surface.dimensionChanged || data->surface.offsetChanged) {
        rebuildSurface(
            &data->surface.controlPoints,
            data->surface.dimension,
            data->surface.resolution,
            data->surface.controlPointOffset,
            data->surface.textureTiling,
            data->surface.minPoint,
            data->surface.maxPoint,
            &data->surface.extremesValid
        );
        data->surface.offsetChanged = false;
        data->surface.dimensionChanged = false;
        data->surface.resolutionChanged = false;

        vec3 center;
        glm_vec3_add(data->surface.controlPoints.data[0],
                    data->surface.controlPoints.data[data->surface.controlPoints.size - 1], center);
        glm_vec3_scale(center, 0.5f, center);
        center[1] += LIGHT_OFFSET_Y;
        glm_vec3_copy(center, data->pointLight.center);

        logic_initCameraFlight(data);
        physics_init();
        updateObstacles(data);
    }

    if (data->surface.resolutionChanged) {
        generateSurfaceVertices(
            &data->surface.controlPoints,
            data->surface.resolution,
            data->surface.dimension,
            data->surface.textureTiling,
            data->surface.minPoint,
            data->surface.maxPoint,
            &data->surface.extremesValid,
            true
        );
        data->surface.resolutionChanged = false;
        logic_initCameraFlight(data);
    }

    // Initialize camera flight when started
    static bool wasFlying = false;
    if (data->cam.isFlying && !wasFlying) {
        logic_initCameraFlight(data);
    }
    wasFlying = data->cam.isFlying;

    // Update camera flight
    if (data->cam.isFlying) {
        logic_updateCameraFlight(data, data->deltaTime);
    }

    physics_update();
}

void logic_printPolynomials(void) {
    printf("\nPOLYNOMIALS\n");
    for (int i = 0; i < g_patches.size; ++i) {
        printf("POLY %d: q(s,t) = ", i + 1);

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float c = g_patches.data[i].coeffsY[col][row];
                if (fabsf(c) < 1e-6f) continue;
                printf("%+.4f*s^%d*t^%d ", c, 3 - row, 3 - col);
            }
        }
        printf("\n");
    }
}

void logic_init(void) {
    PatchArr_init(&g_patches);
}

void logic_cleanup(void) {
    PatchArr_free(&g_patches);
    vec3arr_free(&getInputData()->surface.controlPoints);
    physics_cleanup();
}

void logic_initCameraFlight(InputData *data) {
    vec3 highest, lowest;
    
    // Use cached extremes (always valid at this point)
    glm_vec3_copy(data->surface.maxPoint, highest);
    glm_vec3_copy(data->surface.minPoint, lowest);

    // Set start and end points (with height offset)
    glm_vec3_copy(highest, data->cam.flight.p0);
    glm_vec3_copy(lowest, data->cam.flight.p3);

    // Calculate intermediate control points
    // Line is divided into 3 equal parts
    vec3 line;
    glm_vec3_sub(data->cam.flight.p3, data->cam.flight.p0, line);

    // P1 at 1/3 of the line (x,z only)
    data->cam.flight.p1[0] = data->cam.flight.p0[0] + line[0] / 3.0f;
    data->cam.flight.p1[2] = data->cam.flight.p0[2] + line[2] / 3.0f;

    // P2 at 2/3 of the line (x,z only)
    data->cam.flight.p2[0] = data->cam.flight.p0[0] + 2.0f * line[0] / 3.0f;
    data->cam.flight.p2[2] = data->cam.flight.p0[2] + 2.0f * line[2] / 3.0f;

    // Calculate y coordinates from surface at those x,z positions
    float maxX = data->surface.controlPoints.data[data->surface.dimension-1][0];
    float maxZ = data->surface.controlPoints.data[(data->surface.dimension-1)*data->surface.dimension][2];

    // For P1: convert world coords to normalized surface coords, then evaluate
    float T_s1 = data->cam.flight.p1[2] / maxZ;
    float T_t1 = data->cam.flight.p1[0] / maxX;
    data->cam.flight.p1[1] = evalSurfaceAt(data->surface.dimension, T_s1, T_t1) + CAMERA_HEIGHT_OFFSET;

    // For P2
    float T_s2 = data->cam.flight.p2[2] / maxZ;
    float T_t2 = data->cam.flight.p2[0] / maxX;
    data->cam.flight.p2[1] = evalSurfaceAt(data->surface.dimension, T_s2, T_t2) + CAMERA_HEIGHT_OFFSET;

    // Reset time parameter
    data->cam.flight.t = 0.0f;
}

void logic_updateCameraFlight(InputData *data, float deltaTime) {
    if (!data->cam.isFlying) {
        return;
    }

    // Advance time
    data->cam.flight.t += deltaTime / data->cam.flight.duration;

    if (data->cam.flight.t >= 1.0f) {
        // Flight finished
        data->cam.flight.t = 1.0f;
        data->cam.isFlying = false;
    }

    // Evaluate position on Bezier curve
    utils_evalBezier3D(
        data->cam.flight.p0,
        data->cam.flight.p1,
        data->cam.flight.p2,
        data->cam.flight.p3,
        data->cam.flight.t,
        data->cam.pos
    );

    // Evaluate tangent for camera direction
    utils_evalBezierTangent3D(
        data->cam.flight.p0,
        data->cam.flight.p1,
        data->cam.flight.p2,
        data->cam.flight.p3,
        data->cam.flight.t,
        data->cam.dir
    );
}

void logic_evalSplineGlobal(float gT, float gS, vec3 posDest, vec3 normalDest) {
    InputData *data = getInputData();
    if (data->surface.dimensionChanged || data->surface.resolutionChanged) {
        return;
    }

    int dimension = data->surface.dimension;
    int patchCount = dimension - 3;

    float maxX = data->surface.controlPoints.data[dimension-1][0];
    float maxZ = data->surface.controlPoints.data[(dimension-1)*dimension][2];
    float stepX = maxX / (patchCount * 3.0f);
    float stepZ = maxZ / (patchCount * 3.0f);

    // Convert global params to patch indices and local params
    float global_s = gS * patchCount;
    int patch_s = (int) floor(global_s);
    patch_s = CLAMP(patch_s, 0, patchCount - 1);
    float local_s = global_s - patch_s;

    float global_t = gT * patchCount;
    int patch_t = (int) floor(global_t);
    patch_t = CLAMP(patch_t, 0, patchCount - 1);
    float local_t = global_t - patch_t;

    // Evaluate patch at local coordinates
    Patch *p = &g_patches.data[patch_s * patchCount + patch_t];
    PatchEvalResult res = utils_evalPatchLocal(p, local_s, local_t);

    vec3 pos = {
        (patch_t * 3 + local_t * 3) * stepX,
        res.value,
        (patch_s * 3 + local_s * 3) * stepZ
    };

    glm_vec3_copy(pos, posDest);
    utils_getNormal(res.dsd, res.dtd, stepX, stepZ, normalDest);
}

void logic_closestSplinePointTo(vec3 worldPos, float *outS, float *outT) {
    InputData *data = getInputData();
    int dimension = data->surface.dimension;

    // Get surface bounds
    float maxX = data->surface.controlPoints.data[dimension-1][0];
    float maxZ = data->surface.controlPoints.data[(dimension-1)*dimension][2];

    // Project world pos to normalized coords
    float gT = worldPos[0] / maxX;
    float gS = worldPos[2] / maxZ;
    gT = CLAMP(gT, 0.0f, 1.0f);
    gS = CLAMP(gS, 0.0f, 1.0f);

    *outS = gS;
    *outT = gT;
}
