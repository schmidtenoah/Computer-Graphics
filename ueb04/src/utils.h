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
#define RAND(min, max) ((min) + RAND01 * ((max) - (min)))

/**
 * Defines a dynamic array type with init, free, clear, reserve, push
 * and popBack functions for any given element TYPE.
 *
 * @param TYPE Element type stored in the array
 * @param NAME Name of the generated array type
 */
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
}                                                                              \
                                                                               \
static inline void NAME##_popBack(NAME *arr) {                                 \
    if (arr->size > 0) {                                                       \
        arr->size--;                                                           \
    }                                                                          \
}                                                                              \

void utils_moveTowards(vec3 curr, vec3 target, float speed);

#endif // UTILS_H