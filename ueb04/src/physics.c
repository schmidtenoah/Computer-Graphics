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
#include "instanced.h"

#define NUM_SPHERES 2
#define SPHERE_MAX_WAIT_SEC 10.0f
#define SPHERE_MIN_WAIT_SEC 2.0f
#define SPHERE_SPEED 0.5f

#define SPHERE_COLOR VEC3(0.4f, 0.5f, 1)
#define CENTER_SPHERE_COLOR VEC3(0.3f, 1.0f, 0.3f)

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

    struct {
        vec3 forward;
        vec3 up;
        vec3 right;
    } basis;
} Particle;

DEFINE_ARRAY_TYPE(Particle, ParticleArr);

////////////////////////    LOCAL    ////////////////////////////

/** Global sphere array */
static Sphere g_spheres[NUM_SPHERES] = { 0 };

static ParticleArr g_particles;

/** Manual center position for TM_BOX_CENTER mode */
static vec3 g_manualCenter = {0.0f, 0.0f, 0.0f};

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
                s->waitSec = RAND(SPHERE_MIN_WAIT_SEC, SPHERE_MAX_WAIT_SEC);
            }
        }
    }
}

static void getTargetAcceleration(Particle *p, vec3 target, vec3 dest) {
    vec3 diff;
    glm_vec3_sub(target, p->pos, diff);
    float dist = glm_vec3_norm(diff);

    // Normalize
    if (dist > 1e-5f) {
        glm_vec3_scale(diff, 1.0f / dist, dest);
    } else {
        glm_vec3_copy(VEC3X(0), dest);
    }

    // Scale by kWeak
    glm_vec3_scale(dest, p->kWeak, dest);
}

static void computeAcceleration(TargetMode mode, InputData *data, Particle *p, vec3 dest) {
    glm_vec3_zero(dest);

    switch (mode) {
        case TM_SPHERES: {
            vec3 tempAcc;
            for (int i = 0; i < NUM_SPHERES; i++) {
                Sphere *s = &g_spheres[i];
                getTargetAcceleration(p, s->currPos, tempAcc);

                // Gaussian weighting: g = exp(-dist^2 / const)
                float dist2 = glm_vec3_distance2(s->currPos, p->pos);
                float g = expf(-dist2 / data->particles.gaussianConst);

                glm_vec3_scale(tempAcc, g, tempAcc);
                glm_vec3_add(tempAcc, dest, dest);
            }
            break;
        }

        case TM_LEADER: {
            // In leader mode, non-leaders follow the leader
            int leaderIdx = data->particles.leaderIdx;
            if (leaderIdx >= 0 && leaderIdx < g_particles.size) {
                getTargetAcceleration(p, g_particles.data[leaderIdx].pos, dest);
            }
            break;
        }

        case TM_CENTER: {
            vec3 center = {0, 0, 0};
            for (int i = 0; i < g_particles.size; ++i) {
                glm_vec3_add(center, g_particles.data[i].pos, center);
            }

            glm_vec3_scale(center, 1.0f / g_particles.size, center);
            getTargetAcceleration(p, center, dest);
            break;
        }

        case TM_BOX_CENTER: {
            getTargetAcceleration(p, g_manualCenter, dest);
            break;
        }

        default:
            break;
    }
}

static void applyRoomCollision(InputData *data, Particle *p) {
    float halfSize = data->rendering.roomSize;
    float margin = 0.05f * halfSize;

    vec3 force = { 0, 0, 0 };
    float k = data->physics.roomForce;

    // Check X boundaries
    float dx = p->pos[0];
    if (dx > halfSize - margin) force[0] = -k * (dx - (halfSize - margin)) / margin;
    else if (dx < -halfSize + margin) force[0] = -k * (dx + (halfSize - margin)) / margin;

    // Check Y boundaries
    float dy = p->pos[1];
    if (dy > halfSize - margin) force[1] = -k * (dy - (halfSize - margin)) / margin;
    else if (dy < -halfSize + margin) force[1] = -k * (dy + (halfSize - margin)) / margin;

    // Check Z boundaries
    float dz = p->pos[2];
    if (dz > halfSize - margin) force[2] = -k * (dz - (halfSize - margin)) / margin;
    else if (dz < -halfSize + margin) force[2] = -k * (dz + (halfSize - margin)) / margin;

    // Apply force to velocity
    glm_vec3_scale(force, data->physics.fixedDt, force);
    glm_vec3_add(force, p->velocity, p->velocity);
}

static void updateBasis(Particle *p) {
    vec3 upRef = {0, 1, 0};
    if (glm_vec3_norm2(p->velocity) < 1e-6f) {
        return;
    }

    glm_vec3_normalize_to(p->velocity, p->basis.forward);

    vec3 tmpRight;
    if (glm_vec3_norm2(p->acceleration) < 1e-6f) {
        glm_vec3_cross(p->basis.forward, upRef, tmpRight);
    } else {
        glm_vec3_cross(p->basis.forward, p->acceleration, tmpRight);
        if (glm_vec3_norm2(tmpRight) < 1e-6f) {
            glm_vec3_cross(p->basis.forward, upRef, tmpRight);
        }
    }
    glm_normalize_to(tmpRight, p->basis.right);

    glm_vec3_cross(p->basis.right, p->basis.forward, p->basis.up);
    glm_vec3_normalize(p->basis.up);
}

static void updateParticleInstances(void) {
    vec3 *pos          = malloc(g_particles.size * sizeof(vec3));
    vec3 *acceleration = malloc(g_particles.size * sizeof(vec3));
    vec3 *up           = malloc(g_particles.size * sizeof(vec3));
    vec3 *forward      = malloc(g_particles.size * sizeof(vec3));

    for (int i = 0; i < g_particles.size; ++i) {
        glm_vec3_copy(g_particles.data[i].pos, pos[i]);
        glm_vec3_copy(g_particles.data[i].acceleration, acceleration[i]);
        glm_vec3_copy(g_particles.data[i].basis.up, up[i]);
        glm_vec3_copy(g_particles.data[i].basis.forward, forward[i]);
    }

    instanced_update((int)g_particles.size, pos, acceleration, up, forward);

    free(pos);
    free(acceleration);
    free(up);
    free(forward);
}

static void updateParticles(InputData *data) {
    float dt = data->physics.fixedDt;
    float leaderKv = data->particles.leaderKv;
    bool isLeaderMode = (data->particles.targetMode == TM_LEADER);
    int leaderIdx = data->particles.leaderIdx;

    for (int i = 0; i < g_particles.size; ++i) {
        Particle *p = &g_particles.data[i];
        bool isThisLeader = (isLeaderMode && leaderIdx == i);

        // Leader follows spheres, others follow current target mode
        TargetMode effectiveMode = isThisLeader ? TM_SPHERES : data->particles.targetMode;

        computeAcceleration(effectiveMode, data, p, p->acceleration);

        // 1. Update Velocity based on Acceleration (Euler)
        vec3 deltaA;
        glm_vec3_scale(p->acceleration, dt, deltaA);
        glm_vec3_add(p->velocity, deltaA, p->velocity);

        // 2. Enforce fixed speed (kV)
        glm_vec3_normalize(p->velocity);
        float currentKv = isThisLeader ? leaderKv : p->kV;
        glm_vec3_scale(p->velocity, currentKv, p->velocity);

        // 3. Apply Room Collision
        applyRoomCollision(data, p);

        // 4. Update Position
        vec3 deltaV;
        glm_vec3_scale(p->velocity, dt, deltaV);
        glm_vec3_add(p->pos, deltaV, p->pos);

        // 5. Update Orientation Basis
        updateBasis(p);
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    InputData *data = getInputData();
    for (int i = 0; i < NUM_SPHERES; ++i) {
        Sphere *s = &g_spheres[i];

        SPHERE_RANDOM_POS(s, data);
        s->waiting = false;
        s->waitSec = RAND(SPHERE_MIN_WAIT_SEC, SPHERE_MAX_WAIT_SEC);
        s->wandering = true;
        glm_vec3_copy(VEC3X(RAND01), s->color);
        data->physics.sphereSpeed = SPHERE_SPEED;
    }

    glm_vec3_zero(g_manualCenter);
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

    updateParticleInstances();
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

void physics_setNewLeader(void) {
    InputData *data = getInputData();
    if (data->particles.count > 0) {
        data->particles.leaderIdx = (int)(RAND01 * (data->particles.count - 1));
    }
}

void physics_moveCenterManual(vec3 delta) {
    InputData *data = getInputData();
    float halfSize = data->rendering.roomSize * 0.9f;

    glm_vec3_add(g_manualCenter, delta, g_manualCenter);

    // Clamp to room bounds
    g_manualCenter[0] = glm_clamp(g_manualCenter[0], -halfSize, halfSize);
    g_manualCenter[1] = glm_clamp(g_manualCenter[1], -halfSize, halfSize);
    g_manualCenter[2] = glm_clamp(g_manualCenter[2], -halfSize, halfSize);
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

    // Draw center sphere if in CENTER mode
    if (data->particles.targetMode == TM_BOX_CENTER) {
        scene_pushMatrix();

        scene_translateV(g_manualCenter);
        scene_scaleV(VEC3X(radius * 0.8f));

        shader_setColor(CENTER_SPHERE_COLOR);
        model_drawSimple(MODEL_SPHERE);

        scene_popMatrix();
    }

    debug_popRenderScope();
}

void physics_drawParticles(void) {
    debug_pushRenderScope("Particles");
    scene_pushMatrix();

    InputData *data = getInputData();

    vec3 scale;
    ModelType model;
    switch (data->particles.sphereVis) {
        case SV_SPHERE:
            model = MODEL_SPHERE;
            glm_vec3_copy(VEC3X(0.1f), scale);
            break;
        case SV_TRIANGLE:
            glDisable(GL_CULL_FACE);
            model = MODEL_TRIANGLE;
            glm_vec3_copy(VEC3(0.3f, 0.05f, 1.0f), scale);
            break;
        case SV_LINE:
        default:
            model = MODEL_LINE;
            glm_vec3_copy(VEC3(0.2f, 0.2f, 1.0f), scale);
            break;
    }

    int leaderIdx = (data->particles.targetMode == TM_LEADER) ? data->particles.leaderIdx : -1;

    shader_setColor(SPHERE_COLOR);
    shader_setSimpleInstanceData(scale, leaderIdx);
    model_drawInstanced(model);

    if (data->particles.visVectors) {
        shader_setParticleVisData(scale);
        model_drawParticleVis();
    }

    if (data->rendering.dropShadows) {
        float groundHeight = -data->rendering.roomSize;
        shader_setDropShadowData(scale, leaderIdx, true, groundHeight);
        model_draw(model, true);
    }

    glEnable(GL_CULL_FACE);
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

        glm_vec3_copy(GLM_YUP, p.basis.up);
        glm_vec3_copy(GLM_ZUP, p.basis.forward);
        glm_vec3_copy(GLM_XUP, p.basis.right);

        ParticleArr_push(&g_particles, p);
    }

    data->particles.count = count;
    physics_setNewLeader();
    instanced_resize(count);
}