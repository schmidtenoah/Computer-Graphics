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
#define SPHERE_MAX_WAIT_SEC 2.5f
#define SPHERE_SPEED 2.0f

#define SPHERE_COLOR_LEADER VEC3(1, 0, 1)
#define SPHERE_COLOR VEC3(0.4f, 0.5f, 1)

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

static void computeAcceleration(TargetMode mode, InputData *data, Particle *p, vec3 dest) {
    switch (mode) {
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
            vec3 center = { 0, 0, 0 };
            for (int i = 0; i < g_particles.size; ++i) {
                glm_vec3_add(center, g_particles.data[i].pos, center);
            }
            glm_vec3_scale(center, 1.0f / g_particles.size, center);
            
            getTargetAcceleration(p, center, dest);
            break;
        }

        default:
            getTargetAcceleration(p, VEC3X(1.0f), dest);
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

static void updateBasis(Particle *p) {
    glm_vec3_normalize_to(p->velocity, p->basis.forward);

    glm_vec3_cross(p->basis.forward, p->acceleration, p->basis.right);
    glm_vec3_normalize(p->basis.right);

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
    bool useLeaderKv = data->particles.targetMode == TM_LEADER;
    bool leaderIdx = data->particles.leaderIdx;

    for (int i = 0; i < g_particles.size; ++i) {
        Particle *p = &g_particles.data[i];

        TargetMode target = (data->particles.targetMode == TM_LEADER && 
            data->particles.leaderIdx == i) ? TM_SPHERES : data->particles.targetMode
        ;
        computeAcceleration(target, data, p, p->acceleration);

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

        updateBasis(p);
    }
}

static void rotateParticle(Particle *p, SphereVis visMode) {
    vec3 ref;
    glm_vec3_copy((visMode == SV_LINE) ? VEC3(1.0f, 0.0f, 0.0f) : VEC3(0.0f, 1.0f, 0.0f), ref);
    vec3 axis;
    glm_vec3_cross(ref, p->basis.forward, axis);

    float dot = glm_clamp(glm_vec3_dot(ref, p->basis.forward), -1.0f, 1.0f);
    float angle = acosf(dot);

    if (glm_vec3_norm2(axis) < 1e-6f) {
        if (dot > 0.0f) {
            scene_rotate(180.0f, 0, 1, 0);
        }
    } else {
        glm_vec3_normalize(axis);
        scene_rotateV(glm_deg(angle), axis);
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
            glm_vec3_copy(VEC3(0.1f, 0.3f, 0.3f), scale);
            break;
        case SV_LINE:
        default:
            model = MODEL_LINE;
            glm_vec3_copy(VEC3(0.2f, 0.2f, 0.2f), scale);
            break;
    }

    /*for (int i = 0; i < g_particles.size; ++i) {
        Particle *p = &g_particles.data[i];
        scene_pushMatrix();

        scene_translateV(p->pos);
        rotateParticle(p, data->particles.sphereVis);
        scene_scaleV(scale);
        shader_setColor((data->particles.targetMode == TM_LEADER &&
             data->particles.leaderIdx == i) ? SPHERE_COLOR_LEADER : SPHERE_COLOR
        );
        model_drawSimple(model);

        scene_popMatrix();
    }*/

    model_drawInstanced(model);

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
        ParticleArr_push(&g_particles, p);
    }

    data->particles.count = count;
    instanced_resize(count);
}
