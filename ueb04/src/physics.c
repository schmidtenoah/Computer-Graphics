/**
 * @file physics.c
 * @brief Implementation of physics with Euler integration
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "physics.h"
#include "input.h"
#include "model.h"
#include "shader.h"
#include "utils.h"

#define NUM_SPHERES 2
#define SPHERE_MAX_WAIT_SEC 2.5f
#define SPHERE_SPEED 2.0f

#define SPHERE_RANDOM_POS(sphere, data) { \
    vec3 pos = {                          \
        (RAND01 - 0.5f) * 1.9f,           \
        (RAND01 - 0.5f) * 1.9f,           \
        (RAND01 - 0.5f) * 1.9f            \
    };                                    \
    glm_vec3_scale(pos,                   \
        data->rendering.roomSize,         \
        sphere->targetPos                 \
    );                                    \
}

/** Sphere data structure */
typedef struct {
    vec3 currPos;
    vec3 targetPos;
    bool waiting;
    float waitSec;
    bool wandering;
    vec3 color;
} Sphere;

////////////////////////    LOCAL    ////////////////////////////

/** Global sphere array */
static Sphere g_spheres[NUM_SPHERES] = { 0 };

/**
 * Checks wall collisions and applies bounce
 */
/*static void checkWallCollisions(Sphere *s, float radius, float damping) {
    // X walls
    if (s->position[0] - radius < ROOM_MIN) {
        s->position[0] = ROOM_MIN + radius;
        s->velocity[0] = -s->velocity[0] * damping;
    }
    if (s->position[0] + radius > ROOM_MAX) {
        s->position[0] = ROOM_MAX - radius;
        s->velocity[0] = -s->velocity[0] * damping;
    }

    // Y walls (floor/ceiling)
    if (s->position[1] - radius < ROOM_MIN) {
        s->position[1] = ROOM_MIN + radius;
        s->velocity[1] = -s->velocity[1] * damping;
    }
    if (s->position[1] + radius > ROOM_MAX) {
        s->position[1] = ROOM_MAX - radius;
        s->velocity[1] = -s->velocity[1] * damping;
    }

    // Z walls
    if (s->position[2] - radius < ROOM_MIN) {
        s->position[2] = ROOM_MIN + radius;
        s->velocity[2] = -s->velocity[2] * damping;
    }
    if (s->position[2] + radius > ROOM_MAX) {
        s->position[2] = ROOM_MAX - radius;
        s->velocity[2] = -s->velocity[2] * damping;
    }
}*/

/**
 * Euler integration step
 */
/*static void integrateEuler(Sphere *s, float dt, float friction) {
    // v += a * dt
    vec3 accelDt;
    glm_vec3_scale(s->acceleration, dt, accelDt);
    glm_vec3_add(s->velocity, accelDt, s->velocity);

    // Apply friction
    glm_vec3_scale(s->velocity, friction, s->velocity);

    // p += v * dt
    vec3 velDt;
    glm_vec3_scale(s->velocity, dt, velDt);
    glm_vec3_add(s->position, velDt, s->position);
}*/

/**
 * Updates all spheres
 */
static void updateSpheres(InputData *data) {
    float dt = data->physics.fixedDt;

    for (int i = 0; i < NUM_SPHERES; i++) {
        Sphere *s = &g_spheres[i];
        if (!s->wandering) {
            continue;
        }

        if (s->waiting) {
            s->waitSec -= dt;
            if (s->waitSec <= 0.0f) {
                s->waiting = false;
                s->waitSec = 0.0f;

                SPHERE_RANDOM_POS(s, data);
            }
        } 
        else {
            utils_moveTowards(s->currPos, s->targetPos, data->physics.sphereSpeed * dt);

            if (glm_vec3_eqv_eps(s->currPos, s->targetPos)) {
                s->waiting = true;
                s->waitSec = RAND01 * SPHERE_MAX_WAIT_SEC;
            }
        }

    }
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    InputData *data = getInputData();
    for (int i = 0; i < NUM_SPHERES; ++i) {
        Sphere *s = &g_spheres[i];

        SPHERE_RANDOM_POS(s, data);
        s->waiting = false;
        s->waitSec = 0.0f;
        s->wandering = true;
        glm_vec3_copy(VEC3X(RAND01), s->color);
        data->physics.sphereSpeed = SPHERE_SPEED;
    }   
}

void physics_update(void) {
    InputData *data = getInputData();
    if (data->paused) {
        return;
    }

    data->physics.dtAccumulator += data->deltaTime;

    while (data->physics.dtAccumulator >= data->physics.fixedDt) {
        updateSpheres(data);
        data->physics.dtAccumulator -= data->physics.fixedDt;
    }
}

void physics_cleanup(void) {
    // Nothing to clean up
}

void physics_toggleWander(void) {
    for (int i = 0; i < NUM_SPHERES; ++i) {
        Sphere *s = &g_spheres[i];

        glm_vec3_copy(VEC3X(0), s->targetPos);
        glm_vec3_copy(VEC3X(RAND01), s->color);
        bool wander = !s->wandering;
        s->wandering = wander;
        s->waitSec = 0.0f;
        s->waiting = wander;
    }
}

void physics_drawSpheres(void) {
    debug_pushRenderScope("Spheres");

    InputData *data = getInputData();
    float radius = data->physics.sphereRadius;

    for (int i = 0; i < NUM_SPHERES; i++) {
        scene_pushMatrix();

        scene_translateV(g_spheres[i].currPos);
        scene_scaleV(VEC3X(radius));

        shader_setColor(g_spheres[i].color);
        model_drawSimple(MODEL_SPHERE);

        scene_popMatrix();
    }

    debug_popRenderScope();
}