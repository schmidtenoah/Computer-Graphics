/**
* @file utils.h
 * @brief Utility macros and helper definitions
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef UTILS_H
#define UTILS_H

#include <fhwcg/fhwcg.h>

/** Vector creation macros */
#define VEC3(x, y, z) ((vec3){(float)(x), (float)(y), (float)(z)})
#define VEC3X(x) ((vec3){(float)(x), (float)(x), (float)(x)})
#define VEC2(x, y) ((vec2){(float)(x), (float)(y)})

/** Math utilities */
#define CLAMP(x, min, max) ((x < min) ? min : (x > max) ? max : x)
#define RAND01 ((float)rand() / RAND_MAX)

#endif // UTILS_H