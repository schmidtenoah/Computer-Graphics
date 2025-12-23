/**
* @file utils.c
 * @brief Minimal utility functions
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "utils.h"
#include <fhwcg/fhwcg.h>

void utils_moveTowards(vec3 curr, vec3 target, float speed) {
    vec3 delta;
    glm_vec3_sub(target, curr, delta);

    float dist = glm_vec3_norm(delta);

    if (dist <= speed) {
        glm_vec3_copy(target, curr);
    } else {
        glm_vec3_scale(delta, speed, delta);
        glm_vec3_add(curr, delta, curr);
    }
}
