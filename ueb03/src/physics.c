/**
 * @file physics.c
 * @brief Physics with Euler-Integration, walls, ball collisions, black holes and goal
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "physics.h"
#include "utils.h"
#include "input.h"
#include "logic.h"
#include "model.h"

#define RAND01 ((float)rand() / RAND_MAX)

#define WALL_CNT 4
#define DEFAULT_BALL_NUM 10
#define DEFAULT_BALL_RADIUS 0.1f
#define DEFAULT_BALL_VELOCITY {0, 0, 0}
#define DEFAULT_BALL_ACCELERATION {0, 0, 0}
#define DEFAULT_BLACKHOLE_COUNT 5
#define GOAL_RADIUS 0.3f

#define DEFAULT_BALL(idx) {                     \
    .velocity = DEFAULT_BALL_VELOCITY,          \
    .acceleration = DEFAULT_BALL_ACCELERATION,  \
    .contact = {                                \
        .s = idx / (float) DEFAULT_BALL_NUM,    \
        .t = idx / (float) DEFAULT_BALL_NUM,    \
    },                                          \
    .active = true                              \
}

typedef struct {
    vec3 position;
} BlackHole;

typedef struct {
    vec3 normal;
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
    bool active;
} Ball;

DEFINE_ARRAY_TYPE(Ball, BallArr);
DEFINE_ARRAY_TYPE(BlackHole, BlackHoleArr);

static BallArr g_balls;
static BlackHoleArr g_blackHoles;

static struct {
    Wall walls[WALL_CNT];
    bool initialized;
} g_walls = {
    .initialized = false
};

static struct {
    vec3 position;
    float radius;
    bool reached;
} g_goal = {
    .radius = GOAL_RADIUS,
    .reached = false
};

static const Material BALL_MAT = {
    .ambient = {0.5f, 0.5f, 0.5f},
    .diffuse = {0.6f, 0.6f, 0.6f},
    .emission = {0.0f, 0.0f, 0.0f},
    .specular = {0.1f, 0.1f, 0.1f},
    .shininess = 200.0f,
    .alpha = 1.0f
};

static const Material BLACKHOLE_MAT = {
    .ambient = {0.0f, 0.0f, 0.0f},
    .diffuse = {0.1f, 0.0f, 0.1f},
    .emission = {0.0f, 0.0f, 0.0f},
    .specular = {0.0f, 0.0f, 0.0f},
    .shininess = 10.0f,
    .alpha = 0.9f
};

static const Material GOAL_MAT = {
    .ambient = {0.0f, 0.5f, 0.0f},
    .diffuse = {0.0f, 0.8f, 0.0f},
    .emission = {0.0f, 0.2f, 0.0f},
    .specular = {0.2f, 0.2f, 0.2f},
    .shininess = 100.0f,
    .alpha = 0.5f
};

////////////////////////    LOCAL    ////////////////////////////

static void initWalls(void) {
    InputData *data = getInputData();
    int dimension = data->surface.dimension;

    float maxX = data->surface.controlPoints.data[dimension-1][0];
    float maxZ = data->surface.controlPoints.data[(dimension-1)*dimension][2];

    glm_vec3_copy((vec3){1, 0, 0}, g_walls.walls[0].normal);
    g_walls.walls[0].distance = 0.0f;

    glm_vec3_copy((vec3){-1, 0, 0}, g_walls.walls[1].normal);
    g_walls.walls[1].distance = maxX;

    glm_vec3_copy((vec3){0, 0, 1}, g_walls.walls[2].normal);
    g_walls.walls[2].distance = 0.0f;

    glm_vec3_copy((vec3){0, 0, -1}, g_walls.walls[3].normal);
    g_walls.walls[3].distance = maxZ;

    g_walls.initialized = true;
}

static void initBlackHoles(void) {
    BlackHoleArr_clear(&g_blackHoles);

    for (int i = 0; i < DEFAULT_BLACKHOLE_COUNT; i++) {
        BlackHole bh;

        float s = 0.05f + RAND01 * 0.95f;
        float t = 0.05f + RAND01 * 0.95f;

        vec3 normal;
        logic_evalSplineGlobal(t, s, bh.position, normal);
        BlackHoleArr_push(&g_blackHoles, bh);
    }
}

static void initGoal(void) {
    InputData *data = getInputData();

    vec3 goalPos, normal;
    if (glm_vec3_eqv_eps(data->surface.minPoint, data->surface.maxPoint)) {
        glm_vec3_copy(
            data->surface.controlPoints.data[data->surface.controlPoints.size - 1], 
            goalPos
        );
    } else {
        glm_vec3_copy(data->surface.minPoint, goalPos);
    }

    // Position slightly above surface for better visibility
    float s, t;
    logic_closestSplinePointTo(goalPos, &s, &t);
    logic_evalSplineGlobal(t, s, g_goal.position, normal);

    g_goal.reached = false;
}

static void applyContactPoint(Ball *b, float radius) {
    assert(b != NULL);

    vec3 center;
    glm_vec3_scale(b->contact.normal, radius, center);
    glm_vec3_add(b->contact.point, center, b->center);
}

static void applyWallPenalty(
    Ball *b, Wall *w, float ballMass, float penetration,
    float springConst, float wallDamping
) {
    assert(b != NULL && w != NULL);
    assert(penetration > 0.0f);

    float counterforce = springConst * penetration;

    vec3 penaltyForce, additionalAcceleration;
    glm_vec3_scale(w->normal, counterforce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / ballMass, additionalAcceleration);

    glm_vec3_add(additionalAcceleration, b->acceleration, b->acceleration);

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

    if (!data->physics.wall.enabled) {
        return;
    }

    float springConst = data->physics.wall.spring;
    float ballRadius = data->physics.ballRadius;
    float ballMass = data->physics.mass;
    float wallDamping = data->physics.wall.damping;

    for (int i = 0; i < WALL_CNT; i++) {
        Wall *wall = &g_walls.walls[i];

        float signedDist = glm_vec3_dot(wall->normal, b->center) + wall->distance;
        float penetrationDepth = ballRadius - signedDist;

        if (penetrationDepth > 0.0f) {
            applyWallPenalty(b, wall, ballMass, penetrationDepth, springConst, wallDamping);
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

    float counterForce = springConst * penetration;

    vec3 penaltyForce, penaltyAccel;
    glm_vec3_scale(normal, counterForce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / mass, penaltyAccel);

    glm_vec3_sub(b1->acceleration, penaltyAccel, b1->acceleration);
    glm_vec3_add(b2->acceleration, penaltyAccel, b2->acceleration);

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

    if (!data->physics.ball.enabled) {
        return;
    }

    float radius = data->physics.ballRadius;
    float springConst = data->physics.ball.spring;
    float ballDamping = data->physics.ball.damping;
    float mass = data->physics.mass;

    for (int i2 = i1 + 1; i2 < g_balls.size; ++i2) {
        Ball *b2 = &g_balls.data[i2];

        if (!b2->active) continue;

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

    float counterforce = springConst * penetration;

    vec3 penaltyForce, additionalAcceleration;
    glm_vec3_scale(normal, counterforce, penaltyForce);
    glm_vec3_scale(penaltyForce, 1.0f / ballMass, additionalAcceleration);

    glm_vec3_add(additionalAcceleration, b->acceleration, b->acceleration);

    float velDotNormal = glm_vec3_dot(b->velocity, normal);
    if (velDotNormal < 0.0f) {
        vec3 reflected;
        glm_vec3_reflect(b->velocity, normal, reflected);
        glm_vec3_scale(reflected, obstacleDamping, b->velocity);
    }
}

static void handleObstacleCollisions(InputData *data, Ball *b) {
    assert(b != NULL);

    if (!data->physics.obs.enabled) {
        return;
    }

    float radius = data->physics.ballRadius;
    float springConst = data->physics.obs.spring;
    float obstacleDamping = data->physics.obs.damping;
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

static void handleBlackHoleAttraction(InputData *data, Ball *b) {
    assert(b != NULL);
    float ballMass = data->physics.mass;
    float captureRadius = data->physics.blackHoleCaptureRadius;
    float holeStrength = data->physics.blackHoleStrength;
    float holeRadius = data->physics.blackHoleRadius;

    for (int i = 0; i < g_blackHoles.size; i++) {
        BlackHole *bh = &g_blackHoles.data[i];

        vec3 toBlackHole;
        glm_vec3_sub(bh->position, b->center, toBlackHole);
        float dist = glm_vec3_norm(toBlackHole);

        // Check if ball has been swallowed
        if (dist < captureRadius) {
            b->active = false;
            return;
        }

        if (dist < holeRadius && dist > 0.0001f) {
            vec3 direction;
            glm_vec3_normalize_to(toBlackHole, direction);

            // Attraction Force: F = strength / dist²
            float forceMagnitude = holeStrength / (dist * dist);

            // accelleration: a = F / m
            vec3 attraction;
            glm_vec3_scale(direction, forceMagnitude / ballMass, attraction);
            glm_vec3_add(b->acceleration, attraction, b->acceleration);
        }
    }
}

static void checkGoalReached(Ball *b) {
    assert(b != NULL);

    if (g_goal.reached) {
        return;
    }

    vec3 toGoal;
    glm_vec3_sub(g_goal.position, b->center, toGoal);
    float dist = glm_vec3_norm(toGoal);

    if (dist < g_goal.radius) {
        g_goal.reached = true;
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

    glm_vec3_scale(b->velocity, friction, b->velocity);

    vec3 vMulDt = {0};
    glm_vec3_scale(b->velocity, dt, vMulDt);
    glm_vec3_add(b->contact.point, vMulDt, b->contact.point);

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
        if (!g_balls.data[i].active) continue;
        applyExternForces(&g_balls.data[i], gravity, mass);
    }

    // apply all collision forces and black hole attraction
    for (int i = 0; i < g_balls.size; ++i) {
        Ball *b = &g_balls.data[i];
        if (!b->active) continue;

        handleWallCollision(data, b);
        handleBallCollisions(data, b, i);
        handleObstacleCollisions(data, b);
        handleBlackHoleAttraction(data, b);
    }

    // integrate with new acceleration
    for (int i = 0; i < g_balls.size; ++i) {
        if (!g_balls.data[i].active) continue;
        applyIntegration(&g_balls.data[i], dt, friction, radius);
        checkGoalReached(&g_balls.data[i]);
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator = 0.0f;
    //float radius = data->physics.ballRadius;
    g_balls.size = DEFAULT_BALL_NUM;

    physics_orderBallsAroundMax();
    initWalls();

    BlackHoleArr_free(&g_blackHoles);
    BlackHoleArr_init(&g_blackHoles);
    initBlackHoles();
    initGoal();
}

void physics_addBall(void) {
    Ball b = DEFAULT_BALL((float) DEFAULT_BALL_NUM * RAND01);

    logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
    applyContactPoint(&b, getInputData()->physics.ballRadius);

    BallArr_push(&g_balls, b);
}

void physics_removeBall(void) {
    BallArr_popBack(&g_balls);
}

void physics_addBlackHole(void) {
    BlackHole bh;

    float s = 0.05f + RAND01 * 0.9f;
    float t = 0.1f + RAND01 * 0.9f;

    vec3 normal;
    logic_evalSplineGlobal(t, s, bh.position, normal);

    BlackHoleArr_push(&g_blackHoles, bh);
}

void physics_removeBlackHole(void) {
    if (g_blackHoles.size > 0) {
        BlackHoleArr_popBack(&g_blackHoles);
    }
}

void physics_update(void) {
    InputData *data = getInputData();
    if (data->game.paused) {
        return;
    }

    data->physics.dtAccumulator += data->deltaTime;

    while (data->physics.dtAccumulator >= data->physics.fixedDt) {
        updateBalls(data);
        data->physics.dtAccumulator -= data->physics.fixedDt;
    }
}

void physics_cleanup(void) {
    BallArr_free(&g_balls);
    BlackHoleArr_free(&g_blackHoles);
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
        if (!g_balls.data[i].active) continue;

        scene_pushMatrix();

        scene_translateV(g_balls.data[i].center);
        scene_scaleV(VEC3X(radius));
        scene_getMV(modelviewMat);

        model_draw(MODEL_SPHERE, &BALL_MAT, showNormals, &viewMat, &modelviewMat);
        scene_popMatrix();
    }

    debug_popRenderScope();
}

void physics_drawBlackHoles(void) {
    debug_pushRenderScope("BlackHoles");

    InputData *data = getInputData();
    bool showNormals = data->showNormals;
    glEnable(GL_BLEND);

    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);

    for (int i = 0; i < g_blackHoles.size; ++i) {
        scene_pushMatrix();

        scene_translateV(g_blackHoles.data[i].position);
        scene_scaleV(VEC3X(data->physics.blackHoleRadius * 0.7f));
        scene_getMV(modelviewMat);

        model_draw(MODEL_SPHERE, &BLACKHOLE_MAT, showNormals, &viewMat, &modelviewMat);
        scene_popMatrix();
    }

    glDisable(GL_BLEND);
    debug_popRenderScope();
}

void physics_drawGoal(void) {
    debug_pushRenderScope("Goal");

    InputData *data = getInputData();
    bool showNormals = data->showNormals;

    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);

    glEnable(GL_BLEND);
    scene_pushMatrix();

    scene_translateV(g_goal.position);
    scene_scaleV(VEC3X(g_goal.radius));
    scene_getMV(modelviewMat);

    model_draw(MODEL_SPHERE, &GOAL_MAT, showNormals, &viewMat, &modelviewMat);
    scene_popMatrix();

    glDisable(GL_BLEND);
    debug_popRenderScope();
}

void physics_orderBallsDiagonally(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator = 0.0f;
    float radius = data->physics.ballRadius;

    int count = (int) g_balls.size;
    BallArr_free(&g_balls);
    BallArr_init(&g_balls);
    BallArr_reserve(&g_balls, count);

    for (int i = 0; i < count; ++i) {
        Ball b = DEFAULT_BALL(i);

        logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
        applyContactPoint(&b, radius);

        BallArr_push(&g_balls, b);
    }

    g_goal.reached = false;
}

void physics_orderBallsRandom(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator = 0.0f;
    float radius = data->physics.ballRadius;

    int count = (int) g_balls.size;
    BallArr_free(&g_balls);
    BallArr_init(&g_balls);
    BallArr_reserve(&g_balls, count);

    for (int i = 0; i < count; ++i) {
        Ball b = DEFAULT_BALL(RAND01 * (count - 1));

        logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
        applyContactPoint(&b, radius);

        BallArr_push(&g_balls, b);
    }

    g_goal.reached = false;
}

void physics_orderBallsAroundMax(void) {
    InputData *data = getInputData();
    data->physics.dtAccumulator = 0.0f;
    float radius = data->physics.ballRadius;
    float spawnRadius = data->physics.ballSpawnRadius;

    int count = (int) g_balls.size;
    BallArr_free(&g_balls);
    BallArr_init(&g_balls);
    BallArr_reserve(&g_balls, count);

    vec3 maxPoint;
    glm_vec3_copy(data->surface.maxPoint, maxPoint);
    maxPoint[0] = glm_clamp(maxPoint[0], 0.5f, 500.0f);
    maxPoint[2] = glm_clamp(maxPoint[2], 0.5f, 500.0f);

    for (int i = 0; i < count; ++i) {
        Ball b = DEFAULT_BALL(i);

        float angle = RAND01 * 2.0f * (float) M_PI;
        float r = RAND01 * spawnRadius;

        float x = maxPoint[0] + r * cosf(angle);
        float z = maxPoint[2] + r * sinf(angle);

        if (x < radius) x = radius;
        if (z < radius) z = radius;
        if (x > maxPoint[0]) x = maxPoint[0] - radius;
        if (z > maxPoint[2]) z = maxPoint[2] - radius;

        b.contact.point[0] = x;
        b.contact.point[2] = z;
        b.contact.point[1] = 0.0f;

        logic_closestSplinePointTo(b.contact.point, &b.contact.s, &b.contact.t);
        logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
        applyContactPoint(&b, radius);

        BallArr_push(&g_balls, b);
    }

    g_goal.reached = false;
}

bool physics_isGameWon(void) {
    return g_goal.reached;
}

bool physics_isGameLost(void) {
    // Game lost when all balls are inside black holes
    int activeBalls = 0;
    for (int i = 0; i < g_balls.size; i++) {
        if (g_balls.data[i].active) {
            activeBalls++;
        }
    }
    return activeBalls == 0 && !g_goal.reached;
}

void physics_resetGame(void) {
    InputData *data = getInputData();

    g_goal.reached = false;
    data->game.paused = true;
    physics_init();
}

int physics_getBallCount(void) {
    int count = 0;
    for (int i = 0; i < g_balls.size; i++) {
        if (g_balls.data[i].active) count++;
    }
    return count;
}

int physics_getBlackHoleCount(void) {
    return (int)g_blackHoles.size;
}

void physics_kickBall(void) {
    InputData *data = getInputData();
    if (data->game.paused || data->paused) {
        return;
    }
    
    Ball *b = NULL;
    for (int i = 0; i < g_balls.size && b == NULL; ++i) {
        if (g_balls.data[i].active) {
            b = &g_balls.data[i];
        }
    }

    if (b == NULL) {
        return;
    }

    float angle = RAND01 * 2.0f * (float) M_PI;
    vec3 dir = {cosf(angle), 0.0f, sinf(angle)};

    vec3 velocityKick;
    glm_vec3_scale(dir, data->physics.kickStrength, velocityKick);
    glm_vec3_add(b->velocity, velocityKick, b->velocity);
}
