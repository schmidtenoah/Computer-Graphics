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
#define ROOM_SIZE 5.0f
#define ROOM_MIN (-ROOM_SIZE / 2.0f)
#define ROOM_MAX (ROOM_SIZE / 2.0f)

/** Sphere data structure */
typedef struct {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 color;
} Sphere;

////////////////////////    LOCAL    ////////////////////////////

/** Global sphere array */
static Sphere g_spheres[NUM_SPHERES];

/**
 * Checks wall collisions and applies bounce
 */
static void checkWallCollisions(Sphere *s, float radius, float damping) {
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
}

/**
 * Euler integration step
 */
static void integrateEuler(Sphere *s, float dt, float friction) {
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
}

/**
 * Updates all spheres
 */
static void updateSpheres(InputData *data) {
    float dt = data->physics.fixedDt;
    vec3 gravity = {0, -data->physics.gravity, 0};
    float friction = data->physics.frictionFactor;
    float radius = data->physics.sphereRadius;
    float damping = data->physics.bounceDamping;

    for (int i = 0; i < NUM_SPHERES; i++) {
        Sphere *s = &g_spheres[i];

        // Apply gravity
        glm_vec3_copy(gravity, s->acceleration);

        // Integrate
        integrateEuler(s, dt, friction);

        // Check collisions
        checkWallCollisions(s, radius, damping);
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    // Sphere 1: top left
    glm_vec3_copy(VEC3(-1.0f, 1.5f, -1.0f), g_spheres[0].position);
    glm_vec3_zero(g_spheres[0].velocity);
    glm_vec3_zero(g_spheres[0].acceleration);
    glm_vec3_copy(VEC3(1.0f, 0.3f, 0.3f), g_spheres[0].color);

    // Sphere 2: top right
    glm_vec3_copy(VEC3(1.0f, 1.5f, 1.0f), g_spheres[1].position);
    glm_vec3_zero(g_spheres[1].velocity);
    glm_vec3_zero(g_spheres[1].acceleration);
    glm_vec3_copy(VEC3(0.3f, 0.3f, 1.0f), g_spheres[1].color);
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

void physics_drawSpheres(void) {
    debug_pushRenderScope("Spheres");

    InputData *data = getInputData();
    float radius = data->physics.sphereRadius;

    for (int i = 0; i < NUM_SPHERES; i++) {
        scene_pushMatrix();

        scene_translateV(g_spheres[i].position);
        scene_scaleV(VEC3X(radius));

        shader_setColor(g_spheres[i].color);
        model_drawSimple(MODEL_SPHERE);

        scene_popMatrix();
    }

    debug_popRenderScope();
}