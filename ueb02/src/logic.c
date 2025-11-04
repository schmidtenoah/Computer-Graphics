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

static struct {
    vec4 coeffsX;
    vec4 coeffsY;
} bezierSegment;

DEFINE_ARRAY_TYPE(Patch, PatchArr)

////////////////////////    LOCAL    ////////////////////////////

static PatchArr g_patches;
static float g_curveT = 0.0f;

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
            vec3arr_push(&newPoints, p);
        }
    }

    vec3arr_free(cp);
    *cp = newPoints;
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

void generateSurfaceVertices(Vec3Arr *cp, int samples, int dimension) {
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

            // TexCoords global
            texcoords[idx][0] = T_s;
            texcoords[idx][1] = T_t;
        }
    }

    model_updateSurface(positions, normals, texcoords, gridSize);
    free(positions);
    free(normals);
    free(texcoords);
}


static void rebuildSurface(Vec3Arr *cp, int dimension, int samples, float cpOffset) {
    updateControlPoints(cp, dimension, cpOffset);
    updatePatchesFromControlPoints(cp, dimension);
    generateSurfaceVertices(cp, samples, dimension);
}

static void checkSelectionState(InputData *data) {
    if (data->selection.pressingUp) {
        data->surface.controlPoints.data[data->selection.selectedCp][1] += data->selection.selectedYChange;
        data->surface.dimensionChanged = true;
    }

    if (data->selection.pressingDown) {
        data->surface.controlPoints.data[data->selection.selectedCp][1] -= data->selection.selectedYChange;
        data->surface.dimensionChanged = true;
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
            data->surface.controlPointOffset
        );
        data->surface.offsetChanged = false;
        data->surface.dimensionChanged = false;
        data->surface.resolutionChanged = false;
    }

    // Only sample new position if resolution changed
    if (data->surface.resolutionChanged) {
        generateSurfaceVertices(
            &data->surface.controlPoints,
            data->surface.resolution, 
            data->surface.dimension
        );
        data->surface.resolutionChanged = false;
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
