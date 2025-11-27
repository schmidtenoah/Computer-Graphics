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
#include "logic.h"

#define VEC3(x, y, z) ((vec3){(float) x, (float) y, (float) z})
#define VEC3X(x) ((vec3){(float) x, (float) x, (float) x})
#define VEC2(x, y) ((vec2) {x, y})
#define CLAMP(x, min, max) ((x < min) ? min : (x > max) ? max : x)

#define DEFINE_ARRAY_TYPE(TYPE, NAME)                                          \
typedef struct {                                                               \
    TYPE *data;                                                                \
    size_t size;                                                               \
    size_t capacity;                                                           \
} NAME;                                                                        \
                                                                               \
static inline void NAME##_init(NAME *arr) {                                    \
    arr->data = NULL;                                                          \
    arr->size = 0;                                                             \
    arr->capacity = 0;                                                         \
}                                                                              \
                                                                               \
static inline void NAME##_free(NAME *arr) {                                    \
    free(arr->data);                                                           \
    arr->data = NULL;                                                          \
    arr->size = 0;                                                             \
    arr->capacity = 0;                                                         \
}                                                                              \
                                                                               \
static inline void NAME##_clear(NAME *arr) {                                   \
    arr->size = 0;                                                             \
}                                                                              \
                                                                               \
static inline void NAME##_reserve(NAME *arr, size_t min_capacity) {            \
    if (arr->capacity < min_capacity) {                                        \
        size_t new_cap = arr->capacity ? arr->capacity * 2 : 8;                \
        if (new_cap < min_capacity) new_cap = min_capacity;                    \
        TYPE *new_data = malloc(new_cap * sizeof(TYPE));                       \
        assert(new_data && "malloc failed in " #NAME "_reserve");              \
        if (arr->data) memcpy(new_data, arr->data, arr->size * sizeof(TYPE));  \
        free(arr->data);                                                       \
        arr->data = new_data;                                                  \
        arr->capacity = new_cap;                                               \
    }                                                                          \
}                                                                              \
                                                                               \
static inline void NAME##_push(NAME *arr, TYPE value) {                        \
    if (arr->size >= arr->capacity)                                            \
        NAME##_reserve(arr, arr->size + 1);                                    \
    arr->data[arr->size++] = value;                                            \
}

typedef enum {
    HF_FLAT,
    HF_SIN,
    HF_COS,
    HF_GAUSS,
    HF_RANDOM,
    HF_HILL,
    HF_EXP,
    HF_COUNT
} HeightFuncType;

typedef struct {
    vec3 p0, p1, p2, p3;  // 4 control points
    bool isActive;
    float t;              // current time parameter [0,1]
    float duration;       // total duration in seconds
} BezierCameraPath;

/**
 * Initializes a vec3 array.
 * @param arr Pointer to the vec3 array.
 */
static inline void vec3arr_init(Vec3Arr *arr) {
    arr->size = 0;
    arr->capacity = 0;
    arr->data = NULL;
}

/**
 * Frees a vec3 array.
 * @param arr Pointer to the vec3 array.
 */
static inline void vec3arr_free(Vec3Arr *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

/**
 * Resets a vec3 array.
 * @warning Does not free the elements.
 * @param arr Pointer to the vec3 array.
 */
static inline void vec3arr_clear(Vec3Arr *arr) {
    arr->size = 0;
}

/**
 * Reserves the given amount for elements of a vec3 array.
 * Does not reduce the current capacity!
 * @param arr Pointer to the vec3 array.
 * @param min_capacity The new capacity of the array.
 */
static inline void vec3arr_reserve(Vec3Arr *arr, size_t min_capacity) {
    if (arr->capacity < min_capacity) {
        size_t new_cap = arr->capacity ? arr->capacity * 2 : 8;
        if (new_cap < min_capacity)
            new_cap = min_capacity;
        vec3 *new_data = (vec3*) malloc(new_cap * sizeof(vec3));
        assert(new_data && "vec3arr_reserve: malloc failed");
        if (arr->data) {
            memcpy(new_data, arr->data, arr->size * sizeof(vec3));
            free(arr->data);
        }
        arr->data = new_data;
        arr->capacity = new_cap;
    }
}

/**
 * Adds a new element to the vec3 array.
 * @param arr Pointer to the vec3 array.
 * @param value The new Element of the array.
 */
static inline void vec3arr_push(Vec3Arr *arr, vec3 value) {
    if (arr->size >= arr->capacity) {
        vec3arr_reserve(arr, arr->size + 1);
    }
    glm_vec3_copy(value, arr->data[arr->size++]);
}

/**
 * Applies the given heigth modification to all control points of the surface.
 * @param funcType The Type of the Height-Function. 
 */
void utils_applyHeightFunction(HeightFuncType funcType);

/**
 * Calculates the polynomial for the given control points and 
 * saves it in the given patch.
 * Uses the 2D B-Spline interpolation (M * G * M^T).
 * @param p Pointer to a Patch where the result is saved.
 * @param geometryTerm Pointer to a Patch where the result is saved.
 */
void utils_calculatePolynomialPatch(Patch *p, mat4 geometryTerm);

/**
 * Evaluates the given patch at a specific local t and s.
 * Calculates the 2D B-Spline for t,s and both partial derivatives.
 * => (s^T * C * t) where C := Patch (M * G * M^T).
 * @param p Pointer to the Polynomial Patch.
 * @param s Local s value.
 * @param t Local t value.
 * @returns the 2D B-Spline for t,s and both partial derivatives.
 */
PatchEvalResult utils_evalPatchLocal(Patch *p, float s, float t);

/**
 * Bezier Curve evaluation for a vec3.
 * @param p0, p1, p2, p3 The Control Points for the bezier interpolation.
 * @param t The local t value for the sample.
 * @param out The result of the sample.
 */
void utils_evalBezier3D(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t, vec3 out);

/**
 * Bezier Curve tangent evaluation for a vec3.
 * @param p0, p1, p2, p3 The Control Points for the bezier interpolation.
 * @param t The local t value for the sample.
 * @param out The tangent at the given sample position.
 */
void utils_evalBezierTangent3D(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t, vec3 out);

/**
 * Rotates a point around the Y-axis.
 * @param[in,out] currPos pointer to the center point to rotate
 * @param[in,out] currAngle pointer to the current angle (in radians)
 * @param center the center of rotation
 * @param radius the radius of the rotation
 * @param speed the rotation speed (radians per second)
 * @param deltaTime the time elapsed since the last update (in seconds)
 */
void utils_rotateAroundYAxis(vec3* currPos, float* currAngle, vec3 center, float radius, float speed, float deltaTime);

void utils_getNormal(float dsd, float dtd, float stepX, float stepZ, vec3 dest);

#endif // UTILS_H
