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

#define RAND_IN_BOX(dst, boxSize) {       \
    vec3 pos = {                          \
        (RAND01 - 0.5f) * 1.9f,           \
        (RAND01 - 0.5f) * 1.9f,           \
        (RAND01 - 0.5f) * 1.9f            \
    };                                    \
    glm_vec3_scale(pos, boxSize, dst);    \
}

#define RAND_DIR(dst) {                                     \
    vec3 dir = { RAND(-1, 1), RAND(-1, 1), RAND(-1, 1) };   \
    glm_vec3_normalize_to(dir, dst);                        \
}

#define SPHERE_RANDOM_POS(sphere, data) RAND_IN_BOX(sphere->targetPos, data->rendering.roomSize)

/** Sphere data structure */
typedef struct {
    vec3 currPos;
    vec3 targetPos;
    bool waiting;
    float waitSec;
    bool wandering;
    vec3 color;
} Sphere;

typedef struct {
    vec3 pos;
    float kWeak, kV;
    vec3 acceleration;
    vec3 velocity;
} Particle;

DEFINE_ARRAY_TYPE(Particle, ParticleArr);

////////////////////////    LOCAL    ////////////////////////////

/** Global sphere array */
static Sphere g_spheres[NUM_SPHERES] = { 0 };

static ParticleArr g_particles;

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

static void getTargetAcceleration(Particle *p, vec3 target, vec3 dest) {
    glm_vec3_sub(target, p->pos, dest);
    glm_vec3_normalize(dest);
    glm_vec3_scale(dest, p->kWeak, dest);
}

static void computeAcceleration(InputData *data, Particle *p, vec3 dest) {
    switch (data->particles.targetMode) {
        case TM_SPHERES: {
            vec3 temp;
            for (int i = 0; i < NUM_SPHERES; i++) {
                Sphere *s = &g_spheres[i];
                getTargetAcceleration(p, s->currPos, temp);
                float dist2 = glm_vec3_distance2(s->currPos, p->pos);
                float g = expf(- dist2/data->particles.gaussianConst);
                glm_vec3_scale(temp, g, temp);
                glm_vec3_add(temp, dest, dest);
            }
            break;
        }

        case TM_LEADER: {
            getTargetAcceleration(p, g_particles.data[data->particles.leaderIdx].pos, dest);
            break;
        }

        case TM_CENTER: {
            vec3 center;
            for (int i = 0; i < g_particles.size; ++i) {
                glm_vec3_add(center, g_particles.data[i].pos, center);
            }
            glm_vec3_sub(center, p->pos, dest);
            glm_vec3_normalize(dest);
            glm_vec3_scale(dest, p->kWeak / g_particles.size, dest);
            break;
        }

        default:
            glm_vec3_copy(VEC3X(1.0f), dest);
            break;
    }
}

static void applyRoomCollision(InputData *data, Particle *p) {
    float halfSize = 0.5f * data->rendering.roomSize;
    float margin = 0.05f * halfSize;

    vec3 force = { 0, 0, 0 };
    float k = data->physics.roomForce;

    float dx = p->pos[0];
    if (dx > halfSize - margin) force[0] = -k * (dx - (halfSize - margin)) / margin;
    else if (dx < -halfSize + margin) force[0] = -k * (dx + (halfSize - margin)) / margin;

    float dy = p->pos[1];
    if (dy > halfSize - margin) force[1] = -k * (dy - (halfSize - margin)) / margin;
    else if (dy < -halfSize + margin) force[1] = -k * (dy + (halfSize - margin)) / margin;

    float dz = p->pos[2];
    if (dz > halfSize - margin) force[2] = -k * (dz - (halfSize - margin)) / margin;
    else if (dz < -halfSize + margin) force[2] = -k * (dz + (halfSize - margin)) / margin;

    glm_vec3_scale(force, data->physics.fixedDt, force);
    glm_vec3_add(force, p->velocity, p->velocity);
}

static void updateParticles(InputData *data) {
    float dt = data->physics.fixedDt;
    float leaderKv = data->particles.leaderKv;
    bool useLeaderKv = data->particles.targetMode == TM_LEADER;
    bool leaderIdx = data->particles.leaderIdx;

    for (int i = 0; i < g_particles.size; ++i) {
        Particle *p = &g_particles.data[i];

        computeAcceleration(data, p, p->acceleration);

        vec3 deltaA;
        glm_vec3_scale(p->acceleration, dt, deltaA);
        glm_vec3_add(p->velocity, deltaA, p->velocity);
        glm_vec3_normalize(p->velocity);
        float kV = (useLeaderKv && leaderIdx == i) ? leaderKv : p->kV;
        glm_vec3_scale(p->velocity, kV, p->velocity);

        applyRoomCollision(data, p);

        vec3 deltaV;
        glm_vec3_scale(p->velocity, dt, deltaV);
        glm_vec3_add(p->pos, deltaV, p->pos);
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

    physics_updateParticleCount(data->particles.count);
}

void physics_update(void) {
    InputData *data = getInputData();
    if (data->paused) {
        return;
    }

    data->physics.dtAccumulator += data->deltaTime * data->physics.simulationSpeed;

    while (data->physics.dtAccumulator >= data->physics.fixedDt) {
        updateSpheres(data);
        updateParticles(data);
        data->physics.dtAccumulator -= data->physics.fixedDt;
    }
}

void physics_cleanup(void) {
    ParticleArr_free(&g_particles);
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

void physics_drawParticles(void) {
    debug_pushRenderScope("Particles");
    scene_pushMatrix();

    //InputData *data = getInputData();

    for (int i = 0; i < g_particles.size; ++i) {
        Particle *p = &g_particles.data[i];
        scene_pushMatrix();

        scene_translateV(p->pos);
        scene_scale(0.1f, 0.1f, 0.1f);
        model_drawSimple(MODEL_SPHERE);

        scene_popMatrix();
    }

    scene_popMatrix();
    debug_popRenderScope();
}


void physics_updateParticleCount(int count) {
    InputData *data = getInputData();
    float roomSize = data->rendering.roomSize;
    ParticleArr_free(&g_particles);
    ParticleArr_init(&g_particles);
    ParticleArr_reserve(&g_particles, count);

    for (int i = 0; i < count; ++i) {
        Particle p = { 0 };
        RAND_IN_BOX(p.pos, roomSize);
        RAND_DIR(p.velocity);
        p.kWeak = RAND(0.5f, 10.0f);
        p.kV = RAND(1.0f, 2.0f);
        ParticleArr_push(&g_particles, p);
    }

    data->particles.count = count;
}