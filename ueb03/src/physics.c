/**
 * @file physics.c
 * @brief TODO
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "physics.h"
#include "utils.h"
#include "input.h"
#include "logic.h"
#include "model.h"

#define DEFAULT_BALL_NUM 10
#define DEFAULT_BALL_RADIUS 0.1f
#define DEFAULT_BALL_VELOCITY {0, 1, 0}
#define DEFAULT_BALL_ACCELERATION {0, 0, 0}
#define DEFAULT_BALL_ROLL_DIR {0, 1, 0}

#define DEFAULT_BALL(idx) {                     \
    .velocity = DEFAULT_BALL_VELOCITY,          \
    .rollDir = DEFAULT_BALL_ROLL_DIR,           \
    .radius = DEFAULT_BALL_RADIUS,              \
    .acceleration = DEFAULT_BALL_ACCELERATION,  \
    .contact = {                                \
        .s = idx / (float) DEFAULT_BALL_NUM,    \
        .t = idx / (float) DEFAULT_BALL_NUM,    \
        .valid = true                           \
    }                                           \
}

typedef struct {
    vec3 point;
    vec3 normal;

    float s, t;
    bool valid;
} ContactInfo;

typedef struct {
    vec3 center;
    vec3 velocity;
    vec3 acceleration;
    vec3 rollDir;
    
    ContactInfo contact;
    float radius;
} Ball;

DEFINE_ARRAY_TYPE(Ball, BallArr);

static BallArr g_balls;

////////////////////////    LOCAL    ////////////////////////////

static void applyContactPoint(Ball *b) {
    assert(b != NULL);

    vec3 center;
    glm_vec3_scale(b->contact.normal, b->radius, center);
    glm_vec3_add(b->contact.point, center, b->center);
}

////////////////////////    PUBLIC    ////////////////////////////

void physics_init(void) {
    BallArr_free(&g_balls);
    BallArr_init(&g_balls);
    BallArr_reserve(&g_balls, DEFAULT_BALL_NUM);

    for (int i = 0; i < DEFAULT_BALL_NUM; ++i) {
        Ball b = DEFAULT_BALL(i);

        logic_evalSplineGlobal(b.contact.t, b.contact.s, b.contact.point, b.contact.normal);
        applyContactPoint(&b);

        BallArr_push(&g_balls, b);
    }
}

void physics_update(void) {
    InputData *data = getInputData();
    float dt = data->deltaTime;
    //float dt = data->physics.fixedDt;
    float g = data->physics.gravity;
    vec3 gravity = {0, -g, 0};

    for (int i = 0; i < g_balls.size; ++i) {
        Ball *b = &g_balls.data[i];

        float gDotN = glm_vec3_dot(gravity, b->contact.normal);

        vec3 l = {0};
        glm_vec3_scale(b->contact.normal, gDotN, l);

        vec3 f = {0};
        glm_vec3_sub(gravity, l, f);

        vec3 a = {0};
        glm_vec3_scale(a, 1.0f / data->physics.mass, a);

        glm_vec3_add(b->velocity, a, b->velocity);

        vec3 vMulDt = {0};
        glm_vec3_mul(b->velocity, VEC3X(dt), vMulDt);
        glm_vec3_add(b->contact.point, vMulDt, b->contact.point);

        logic_closestSplinePointTo(b->contact.point, &b->contact.s, &b->contact.t);
        logic_evalSplineGlobal(b->contact.t, b->contact.s, b->contact.point, b->contact.normal);

        applyContactPoint(b);
    }
}

void physics_cleanup(void) {
    BallArr_free(&g_balls);
}

void physics_drawBalls(void) {
    assert(g_balls.data != NULL);
    debug_pushRenderScope("Balls");

    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);

    for (int i = 0; i < g_balls.size; ++i) {
        scene_pushMatrix();

        scene_translateV(g_balls.data[i].center);
        scene_scaleV(VEC3X(g_balls.data[i].radius));
        scene_getMV(modelviewMat);

        model_draw(MODEL_SPHERE, getInputData()->showNormals, true, &viewMat, &modelviewMat);
        scene_popMatrix();
    }

    debug_popRenderScope();
}