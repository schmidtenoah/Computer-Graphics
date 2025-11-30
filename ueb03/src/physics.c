/**
 * @file physics.c
 * @brief Physik mit Euler-Integration, Wand- und Kugelkollisionen (Penalty-Methode)
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "physics.h"
#include "utils.h"
#include "input.h"
#include "logic.h"
#include "model.h"

#define WALL_CNT 4
#define DEFAULT_BALL_NUM 10
#define DEFAULT_BALL_RADIUS 0.1f
#define DEFAULT_BALL_VELOCITY {0, 0, 0}
#define DEFAULT_BALL_ACCELERATION {0, 0, 0}

#define DEFAULT_BALL(idx) {                     \
    .velocity = DEFAULT_BALL_VELOCITY,          \
    .acceleration = DEFAULT_BALL_ACCELERATION,  \
    .contact = {                                \
        .s = idx / (float) DEFAULT_BALL_NUM,    \
        .t = idx / (float) DEFAULT_BALL_NUM,    \
    }                                           \
}

typedef struct {
    vec3 normal;

    // from origin along the normals
    float distance;
} Wall;

typedef struct {
    vec3 point;
    vec3 normal;
    float s, t;
} ContactInfo;

typedef struct {
    vec3 center;
    vec3 acceleration;
    vec3 velocity;
    ContactInfo contact;
} Ball;

DEFINE_ARRAY_TYPE(Ball, BallArr);

static BallArr g_balls;

static struct {
    Wall walls[WALL_CNT];
    bool initialized;
} g_walls = {
    .initialized = false
};

static const Material BALL_MAT = {
    .ambient = {0.5f, 0.5f, 0.5f},
    .diffuse = {0.6f, 0.6f, 0.6f},
    .emission = {0.0f, 0.0f, 0.0f},
    .specular = {0.1f, 0.1f, 0.1f},
    .shininess = 200.0f
};

////////////////////////    LOCAL    ////////////////////////////

/**
 * Initialisiert die Wände basierend auf der Splinefläche - Achsenparallel
 */
static void initWalls(void) {
    InputData *data = getInputData();
    int dimension = data->surface.dimension;

    // Grenzen der Splinefläche
    float maxX = data->surface.controlPoints.data[dimension-1][0];
    float maxZ = data->surface.controlPoints.data[(dimension-1)*dimension][2];

    // Linke Wand (X = 0, Normale goes rechts)
    glm_vec3_copy((vec3){1, 0, 0}, g_walls.walls[0].normal);
    g_walls.walls[0].distance = 0.0f;

    // Rechte Wand (X = maxX, Normale goes links)
    glm_vec3_copy((vec3){-1, 0, 0}, g_walls.walls[1].normal);
    g_walls.walls[1].distance = maxX;

    // Vordere Wand (Z = 0, Normale goes hinten)
    glm_vec3_copy((vec3){0, 0, 1}, g_walls.walls[2].normal);
    g_walls.walls[2].distance = 0.0f;

    // Hintere Wand (Z = maxZ, Normale goes vorne)
    glm_vec3_copy((vec3){0, 0, -1}, g_walls.walls[3].normal);
    g_walls.walls[3].distance = maxZ;

    g_walls.initialized = true;
}

static void applyContactPoint(Ball *b, float radius) {
    assert(b != NULL);

    vec3 center;
    glm_vec3_scale(b->contact.normal, radius, center);
    glm_vec3_add(b->contact.point, center, b->center);
}

/**
 * Applies the acceleration penalty and velocity reflection for the given ball and wall.
 * @param b pointer to the ball
 * @param w pointer to the wall
 * @param ballMass the mass of the given ball
 * @param penetration the depth of the penetration
 * @param springConst constant for the spring force
 * @param wallDamping damping for the reflection
 */
static void applyWallPenalty(
    Ball *b, Wall *w, float ballMass, float penetration, 
    float springConst, float wallDamping
) {
    assert(b != NULL && w != NULL);
    assert(penetration > 0.0f);
    
    // f = k * d
    float counterforce = springConst * penetration;

    // acc = (f * normal) / mass
    vec3 penaltyForce, additionalAcceleration;
    glm_vec3_scale(w->normal, counterforce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / ballMass, additionalAcceleration);

    glm_vec3_add(additionalAcceleration, b->acceleration, b->acceleration);

    // velocity correction via reflection, only when ball rolls towards the wall
    float velDotNormal = glm_vec3_dot(b->velocity, w->normal);
    if (velDotNormal < 0.0f) {
        vec3 reflected;
        glm_vec3_reflect(b->velocity, w->normal, reflected);
        glm_vec3_scale(reflected, wallDamping, b->velocity);
    }
}

static void handleWallCollision(InputData *data, Ball *b) {
    assert(b != NULL);
    assert(g_walls.initialized);

    float springConst = data->physics.wallSpringConst;
    float ballRadius = data->physics.ballRadius;
    float ballMass = data->physics.mass;
    float wallDamping = data->physics.wallDamping;

    for (int i = 0; i < WALL_CNT; i++) {
        Wall *wall = &g_walls.walls[i];

        // d = normal · center + distance
        // calcs distance from ball center to wall
        float signedDist = glm_vec3_dot(wall->normal, b->center) + wall->distance;
        float penetrationDepth = ballRadius - signedDist;

        if (penetrationDepth > 0.0f) {
            applyWallPenalty(b,  wall, ballMass, penetrationDepth, springConst, wallDamping);
        }
    }
}

static void applyBallPenalty(
    Ball *b1, Ball *b2, float penetration, vec3 b1ToB2,
    float springConst, float mass, float ballDamping
) {
    assert(b1 != NULL && b2 != NULL);
    assert(penetration > 0.0f);

    vec3 normal;
    glm_vec3_normalize_to(b1ToB2, normal);

    // f = k * d
    float counterForce = springConst * penetration;

    // acc = (f * normal) / mass
    vec3 penaltyForce, penaltyAccel;
    glm_vec3_scale(normal, counterForce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / mass, penaltyAccel);

    // apply accelaration 
    glm_vec3_sub(b1->acceleration, penaltyAccel, b1->acceleration);
    glm_vec3_add(b2->acceleration, penaltyAccel, b2->acceleration);

    // velocity correction via reflection, only when balls rolls towards each other
    vec3 relativeVelocity;
    glm_vec3_sub(b1->velocity, b2->velocity, relativeVelocity);
    float velocityAlongNormal = glm_vec3_dot(relativeVelocity, normal);
    if (velocityAlongNormal > 0.0f) {
        float impulseScale = (1.0f + ballDamping) * velocityAlongNormal * 0.5f;
        vec3 impulse;
        glm_vec3_scale(normal, impulseScale, impulse);

        glm_vec3_sub(b1->velocity, impulse, b1->velocity);
        glm_vec3_add(b2->velocity, impulse, b2->velocity);
    }
}

static void handleBallCollisions(InputData *data, Ball *b1, int i1) {
    assert(b1 != NULL);

    float radius = data->physics.ballRadius;
    float springConst = data->physics.ballSpringConst;
    float ballDamping = data->physics.ballDamping;
    float mass = data->physics.mass;

    for (int i2 = i1 + 1; i2 < g_balls.size; ++i2) {
        Ball *b2 = &g_balls.data[i2];

        vec3 b1ToB2;
        glm_vec3_sub(b2->center, b1->center, b1ToB2);

        float dist = glm_vec3_norm(b1ToB2);
        float diameter = 2.0f * radius;
        float penetrationDepth = diameter - dist;

        if (penetrationDepth > 0.0f && dist > 0.0001f) {
            applyBallPenalty(
                b1, b2, penetrationDepth, 
                b1ToB2, springConst, mass, ballDamping
            );
        }
    }
}

static void applyObstaclePenalty(
    Ball *b, Obstacle *o, float dist, vec3 diff, float springConst,
    float penetration, float ballMass, float obstacleDamping
) {
    assert(b != NULL && o != NULL);

    vec3 normal;
    utils_getAABBNormal(o, b->center, dist, diff, normal);

    // f = k * d
    float counterforce = springConst * penetration;

    // acc = (f * normal) / mass
    vec3 penaltyForce, additionalAcceleration;
    glm_vec3_scale(normal, counterforce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / ballMass, additionalAcceleration);

    glm_vec3_add(additionalAcceleration, b->acceleration, b->acceleration);

    // velocity correction via reflection, only when ball rolls towards the box
    float velDotNormal = glm_vec3_dot(b->velocity, normal);
    if (velDotNormal < 0.0f) {
        vec3 reflected;
        glm_vec3_reflect(b->velocity, normal, reflected);
        glm_vec3_scale(reflected, obstacleDamping, b->velocity);

        /*vec3 impulse;
        glm_vec3_scale(normal, - (1.0f + obstacleDamping) * velDotNormal * 0.5f, impulse);
        glm_vec3_add(b->velocity, impulse, b->velocity);*/
    }
}

static void handleObstacleCollisions(InputData *data, Ball *b) {
    assert(b != NULL);

    float radius = data->physics.ballRadius;
    float springConst = data->physics.obstacleSpringConst;
    float obstacleDamping = data->physics.obstacleDamping;
    float mass = data->physics.mass;

    for (int i = 0; i < data->game.obstacleCnt; i++) {
        Obstacle *o = &data->game.obstacles[i];

        vec3 closest;
        utils_closestPointOnAABB(b->center, o, closest);

        vec3 diff;
        glm_vec3_sub(b->center, closest, diff);
        float dist = glm_vec3_norm(diff);
        float penetrationDepth = radius - dist;

        if (penetrationDepth > 0.0f) {
            applyObstaclePenalty(
                b, o, dist, diff, springConst, 
                penetrationDepth, mass, obstacleDamping
            );
        }
    }
}

static void applyExternForces(Ball *b, vec3 gravity, float mass) {
    assert(b != NULL);

    vec3 l, f;
    float gDotN = glm_vec3_dot(gravity, b->contact.normal);
    glm_vec3_scale(b->contact.normal, gDotN, l);
    glm_vec3_sub(gravity, l, f);

    glm_vec3_scale(f, 1.0f / mass, b->acceleration);
}

static void applyIntegration(Ball *b, float dt, float friction, float ballRadius) {
    assert(b != NULL);

    vec3 accelDt;
    glm_vec3_scale(b->acceleration, dt, accelDt);
    glm_vec3_add(b->velocity, accelDt, b->velocity);

    // apply friction
    glm_vec3_scale(b->velocity, friction, b->velocity);

    // update position: s = s + v * dt
    vec3 vMulDt = {0};
    glm_vec3_scale(b->velocity, dt, vMulDt);
    glm_vec3_add(b->contact.point, vMulDt, b->contact.point);

    // apply new position
    logic_closestSplinePointTo(b->contact.point, &b->contact.s, &b->contact.t);
    logic_evalSplineGlobal(b->contact.t, b->contact.s, b->contact.point, b->contact.normal);
    applyContactPoint(b, ballRadius);
}

static void updateBalls(InputData *data) {
    float dt = data->physics.fixedDt;
    vec3 gravity = {0, -data->physics.gravity, 0};
    float friction = data->physics.frictionFactor;
    float radius = data->physics.ballRadius;
    float mass = data->physics.mass;

    // update acceleration
    for (int i = 0; i < g_balls.size; ++i) {
        applyExternForces(&g_balls.data[i], gravity, mass);
    }

    // apply all collision forces
    for (int i = 0; i < g_balls.size; ++i) {
        Ball *b = &g_balls.data[i];

        handleWallCollision(data, b);
        handleBallCollisions(data, b, i);
        handleObstacleCollisions(data, b);
    }

    // integrate with new acceleration
    for (int i = 0; i < g_balls.size; ++i) {
        applyIntegration(&g_balls.data[i], dt, friction, radius);
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator = 0.0f;
    float radius = data->physics.ballRadius;

    BallArr_free(&g_balls);
    BallArr_init(&g_balls);
    BallArr_reserve(&g_balls, DEFAULT_BALL_NUM);

    for (int i = 0; i < DEFAULT_BALL_NUM; ++i) {
        Ball b = DEFAULT_BALL(i);

        logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
        applyContactPoint(&b, radius);

        BallArr_push(&g_balls, b);
    }

    initWalls();
}

void physics_addBall(void) {
    Ball b = DEFAULT_BALL((float) DEFAULT_BALL_NUM * 0.5f);

    logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
    applyContactPoint(&b, getInputData()->physics.ballRadius);

    BallArr_push(&g_balls, b);
}

void physics_removeBall(void) {
    BallArr_popBack(&g_balls);
}

void physics_update(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator += data->deltaTime;

    while (data->physics.dtAccumulator >= data->physics.fixedDt) {
        updateBalls(data);
        data->physics.dtAccumulator -= data->physics.fixedDt;
    }
}

void physics_cleanup(void) {
    BallArr_free(&g_balls);
    g_walls.initialized = false;
}

void physics_drawBalls(void) {
    assert(g_balls.data != NULL);
    debug_pushRenderScope("Balls");

    InputData *data = getInputData();
    bool showNormals = data->showNormals;
    float radius = data->physics.ballRadius;

    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);

    for (int i = 0; i < g_balls.size; ++i) {
        scene_pushMatrix();

        scene_translateV(g_balls.data[i].center);
        scene_scaleV(VEC3X(radius));
        scene_getMV(modelviewMat);

        model_draw(MODEL_SPHERE, &BALL_MAT, showNormals, &viewMat, &modelviewMat);
        scene_popMatrix();
    }

    debug_popRenderScope();
}
