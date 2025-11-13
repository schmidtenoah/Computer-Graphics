/**
 * @file logic.c
 * @brief Implementation of game logic, physics and level management.
 *
 * Manages all game mechanics including:
 * - Airplane movement along the curve with speed along curve slope.
 * - Collision detection (stars, clouds, airplane vertices)
 * - Level progression and win/lose conditions
 * - Six pre-defined levels with increasing difficulty
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "logic.h"
#include "input.h"
#include "model.h"
#include "utils.h"

#include <fhwcg/fhwcg.h>

#define RANDOM_HEIGHT(scale) ((((float)rand() / RAND_MAX) - 0.5f) * scale)
#define CAMERA_HEIGHT_OFFSET 0.0f // Height that camera flight is above surface

static struct {
    vec4 coeffsX;
    vec4 coeffsY;
} bezierSegment;

DEFINE_ARRAY_TYPE(Patch, PatchArr)

////////////////////////    LOCAL    ////////////////////////////

static PatchArr g_patches;
static float g_curveT = 0.0f;

static void updateControlPoints(Vec3Arr *cp, int newDim, float cpOffset, vec3 minPoint, vec3 maxPoint, bool *extremesValid) {
    int oldDim = (int)sqrtf((float)cp->size);
    if (oldDim * oldDim != (int)cp->size) {
        oldDim = 0;
    }

    float newStep = (1.0f / (newDim - 1)) + cpOffset;
    Vec3Arr newPoints;
    vec3arr_init(&newPoints);
    vec3arr_reserve(&newPoints, newDim * newDim);

    // Initialize extremes
    float minHeight = 1e10f;
    float maxHeight = -1e10f;
    vec3 minPos, maxPos;

    for (int i = 0; i < newDim; ++i) {
        for (int j = 0; j < newDim; ++j) {
            vec3 p;
            p[0] = j * newStep;
            p[2] = i * newStep;

            float height = 0.0f;
            bool reused = false;

            if (i < oldDim && j < oldDim && oldDim > 0) {
                // old point -> take over
                height = cp->data[i * oldDim + j][1];
                reused = true;
            } else if (oldDim > 0) {
                // new point -> random height + neighbors
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
            
            // Track min/max
            if (height > maxHeight) {
                maxHeight = height;
                glm_vec3_copy(p, maxPos);
            }
            if (height < minHeight) {
                minHeight = height;
                glm_vec3_copy(p, minPos);
            }
            
            vec3arr_push(&newPoints, p);
        }
    }

    vec3arr_free(cp);
    *cp = newPoints;
    
    // Store extremes
    glm_vec3_copy(minPos, minPoint);
    glm_vec3_copy(maxPos, maxPoint);
    *extremesValid = true;
}

static void updatePatchesFromControlPoints(Vec3Arr *cp, int dimension) {
    PatchArr_clear(&g_patches);
    Patch patch;

    for (int i = 0; i < dimension - 3; ++i) {
        for (int j = 0; j < dimension - 3; ++j) {
            mat4 geometryTerm = GLM_MAT4_ZERO_INIT;

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

void generateSurfaceVertices(Vec3Arr *cp, int samples, int dimension, float textureTiling) {
    int patchCount = dimension - 3;
    //int gridSize = patchCount * samplesPerPatch + 1;
    int gridSize = (samples < 2) ? 2 : samples;
    int totalVerts = gridSize * gridSize;

    vec3 *positions = malloc(sizeof(vec3) * totalVerts);
    vec3 *normals   = malloc(sizeof(vec3) * totalVerts);
    vec2 *texcoords = malloc(sizeof(vec2) * totalVerts);
    vec3 n;

    for (int i = 0; i < gridSize; ++i) {
        float T_s = (float)i / (gridSize - 1); // global s
        float global_s = T_s * patchCount;
        int patch_s = (int)floor(global_s);
        if (patch_s >= patchCount) patch_s = patchCount - 1;
        float local_s = global_s - patch_s;

        for (int j = 0; j < gridSize; ++j) {
            float T_t = (float)j / (gridSize - 1); // global t
            float global_t = T_t * patchCount;
            int patch_t = (int)floor(global_t);
            if (patch_t >= patchCount) patch_t = patchCount - 1;
            float local_t = global_t - patch_t;

            Patch *p = &g_patches.data[patch_s * patchCount + patch_t];
            PatchEvalResult res = utils_evalPatchLocal(p, local_s, local_t);

            // Index im Vertex-Array
            int idx = i * gridSize + j;

            float maxX = cp->data[dimension-1][0];
            float maxZ = cp->data[(dimension-1)*dimension][2];
            float stepX = maxX / (patchCount * 3.0f);
            float stepZ = maxZ / (patchCount * 3.0f);

            positions[idx][0] = (patch_t * 3 + local_t * 3) * stepX;
            positions[idx][2] = (patch_s * 3 + local_s * 3) * stepZ;
            positions[idx][1] = res.value;

            // Normal
            vec3 rs = { 0.0f, res.dsd, stepZ };
            vec3 rt = { stepX, res.dtd, 0.0f };
            glm_vec3_cross(rs, rt, n);
            glm_vec3_normalize_to(n, normals[idx]);

            // TexCoords with tiling
            texcoords[idx][0] = T_s * textureTiling;
            texcoords[idx][1] = T_t * textureTiling;
        }
    }

    model_updateSurface(positions, normals, texcoords, gridSize);
    free(positions);
    free(normals);
    free(texcoords);
}


static void rebuildSurface(Vec3Arr *cp, int dimension, int samples, float cpOffset, float textureTiling, vec3 minPoint, vec3 maxPoint, bool *extremesValid) {
    updateControlPoints(cp, dimension, cpOffset, minPoint, maxPoint, extremesValid);
    updatePatchesFromControlPoints(cp, dimension);
    generateSurfaceVertices(cp, samples, dimension, textureTiling);
}

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

        // Update camera flight path when surface geometry changes
        logic_initCameraFlight(data);
    }

    // Only sample new position if resolution changed
    if (data->surface.resolutionChanged) {
        generateSurfaceVertices(
            &data->surface.controlPoints,
            data->surface.resolution, 
            data->surface.dimension,
            data->surface.textureTiling
        );
        data->surface.resolutionChanged = false;
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
}

// Evaluate surface at specific global T_s and T_t coordinates
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

void logic_initCameraFlight(InputData *data) {
    vec3 highest, lowest;
    
    // Use cached extremes (always valid at this point)
    glm_vec3_copy(data->surface.maxPoint, highest);
    glm_vec3_copy(data->surface.minPoint, lowest);

    // Set start and end points (with height offset)
    glm_vec3_copy(highest, data->cam.flight.p0);
    data->cam.flight.p0[1] += CAMERA_HEIGHT_OFFSET;

    glm_vec3_copy(lowest, data->cam.flight.p3);
    data->cam.flight.p3[1] += CAMERA_HEIGHT_OFFSET;

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