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

#define DEFAULT_BALL_NUM 10
#define DEFAULT_BALL_RADIUS 0.1f
#define DEFAULT_BALL_VELOCITY {0, 0, 0}
#define DEFAULT_BALL_ACCELERATION {0, 0, 0}
#define DEFAULT_BALL_ROLL_DIR {0, 0, 0}

#define DEFAULT_BALL(idx) {                     \
    .velocity = DEFAULT_BALL_VELOCITY,          \
    .rollDir = DEFAULT_BALL_ROLL_DIR,           \
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
} Ball;

DEFINE_ARRAY_TYPE(Ball, BallArr);

static BallArr g_balls;

// Wand Struct
typedef struct {
    vec3 normal;
    float distance; // Abstand vom Ursprung entlang der Normalen
} Wall;

static Wall g_walls[4];
static bool g_wallsInitialized = false;

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
    glm_vec3_copy((vec3){1, 0, 0}, g_walls[0].normal);
    g_walls[0].distance = 0.0f;

    // Rechte Wand (X = maxX, Normale goes links)
    glm_vec3_copy((vec3){-1, 0, 0}, g_walls[1].normal);
    g_walls[1].distance = maxX;

    // Vordere Wand (Z = 0, Normale goes hinten)
    glm_vec3_copy((vec3){0, 0, 1}, g_walls[2].normal);
    g_walls[2].distance = 0.0f;

    // Hintere Wand (Z = maxZ, Normale goes vorne)
    glm_vec3_copy((vec3){0, 0, -1}, g_walls[3].normal);
    g_walls[3].distance = maxZ;

    g_wallsInitialized = true;
}

/**
 * Penalty-Methode für Wandkollisionen
 */
static void applyWallPenalty(InputData *data, Ball *b, float radius, float mass) {
    assert(g_wallsInitialized);

    for (int i = 0; i < 4; i++) {
        Wall *wall = &g_walls[i];

        // Berechnung signierter Abstand vom Kugelzentrum zur Wand
        // d = normal · center + distance
        float signedDist = glm_vec3_dot(wall->normal, b->center) + wall->distance;

        // Penetrationstiefe: wenn < radius, dann kollidiert die Kugel
        float penetration = radius - signedDist;

        if (penetration > 0.0f) {
            // Penalty-Kraft: f = k * d
            float penaltyForce = data->physics.wallSpringConst * penetration;

            // Kraftvektor: f_vec = f * normal
            vec3 penaltyForceVec;
            glm_vec3_scale(wall->normal, penaltyForce, penaltyForceVec);

            // Beschleunigung: a = f / m
            vec3 penaltyAccel;
            glm_vec3_scale(penaltyForceVec, 1.0f / mass, penaltyAccel);

            // Zur Gesamtbeschleunigung hinzufügen
            glm_vec3_add(b->acceleration, penaltyAccel, b->acceleration);

            // Reflection der Geschwindigkeit an der Wand (inkl. Dämpfung)
            float velDotNormal = glm_vec3_dot(b->velocity, wall->normal);
            if (velDotNormal < 0.0f) {
                vec3 reflected;
                glm_vec3_reflect(b->velocity, wall->normal, reflected);
                glm_vec3_scale(reflected, data->physics.wallDamping, b->velocity);
            }
        }
    }
}

/**
 * Penalty-Methode für Kugel-Kugel-Kollisionen
 */
static void applyBallCollisions(InputData *data, float radius, float mass) {
    for (int i = 0; i < g_balls.size; i++) {
        Ball *b1 = &g_balls.data[i];

        for (int j = i + 1; j < g_balls.size; j++) {
            Ball *b2 = &g_balls.data[j];

            // Vektor von b1 zu b2
            vec3 delta;
            glm_vec3_sub(b2->center, b1->center, delta);

            float dist = glm_vec3_norm(delta);
            float minDist = 2.0f * radius; // same Radius pro Kugel

            float penetration = minDist - dist;

            if (penetration > 0.0f && dist > 0.0001f) {
                // delta für Normale normalisieren
                vec3 normal;
                glm_vec3_normalize_to(delta, normal);

                // Penalty-Kraft
                float penaltyForce = data->physics.ballSpringConst * penetration;

                // Kraftvektor
                vec3 penaltyForceVec;
                glm_vec3_scale(normal, penaltyForce, penaltyForceVec);

                // Beschleunigung
                vec3 penaltyAccel;
                glm_vec3_scale(penaltyForceVec, 1.0f / mass, penaltyAccel);

                // Beschleunigung auf beide Kugeln (Newton's 3. Gesetz)
                glm_vec3_sub(b1->acceleration, penaltyAccel, b1->acceleration);
                glm_vec3_add(b2->acceleration, penaltyAccel, b2->acceleration);

                // Relative Geschwindigkeit -> reflection
                vec3 relVel;
                glm_vec3_sub(b1->velocity, b2->velocity, relVel);

                float velAlongNormal = glm_vec3_dot(relVel, normal);

                // Nur wenn Kugeln aufeinander zu bewegen
                if (velAlongNormal > 0.0f) {
                    float e = data->physics.ballDamping;

                    // Impulsstärke j für gleiche Massen:
                    // v_rel' = -e * v_rel  → j = (1 + e) * v_rel / 2
                    float j_impulse = (1.0f + e) * velAlongNormal * 0.5f;

                    vec3 impulse;
                    glm_vec3_scale(normal, j_impulse, impulse);

                    // Impuls entlang -n auf b1, +n auf b2
                    glm_vec3_sub(b1->velocity, impulse, b1->velocity);
                    glm_vec3_add(b2->velocity, impulse, b2->velocity);
                }
            }
        }
    }
}

static void applyContactPoint(Ball *b, float radius) {
    assert(b != NULL);

    vec3 center;
    glm_vec3_scale(b->contact.normal, radius, center);
    glm_vec3_add(b->contact.point, center, b->center);
}

static void updateBalls(InputData *data) {
    float g = data->physics.gravity;
    float dt = data->physics.fixedDt;
    vec3 gravity = {0, -g, 0};
    float radius = data->physics.ballRadius;
    float mass = data->physics.mass;

    for (int i = 0; i < g_balls.size; ++i) {
        Ball *b = &g_balls.data[i];

        float gDotN = glm_vec3_dot(gravity, b->contact.normal);
        vec3 l = {0};
        glm_vec3_scale(b->contact.normal, gDotN, l);

        vec3 f = {0};
        glm_vec3_sub(gravity, l, f);

        glm_vec3_scale(f, 1.0f / mass, b->acceleration);
    }

    // Kollisionen berechnen (Penalty)
    applyBallCollisions(data, radius, mass);

    // Für jede Kugel: Wandkollisionen und Integration
    for (int i = 0; i < g_balls.size; ++i) {
        Ball *b = &g_balls.data[i];

        // wall collision
        applyWallPenalty(data, b, radius, mass);

        // Euler-Integration: v = v + a * dt
        vec3 accelDt;
        glm_vec3_scale(b->acceleration, dt, accelDt);
        glm_vec3_add(b->velocity, accelDt, b->velocity);

        // apply friction
        glm_vec3_scale(b->velocity, data->physics.frictionFactor, b->velocity);

        // update position: s = s + v * dt
        vec3 vMulDt = {0};
        glm_vec3_scale(b->velocity, dt, vMulDt);
        glm_vec3_add(b->contact.point, vMulDt, b->contact.point);

        // apply new position
        logic_closestSplinePointTo(b->contact.point, &b->contact.s, &b->contact.t);
        logic_evalSplineGlobal(b->contact.t, b->contact.s, b->contact.point, b->contact.normal);
        applyContactPoint(b, radius);
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

    // Wände initialisieren
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
    g_wallsInitialized = false;
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

        model_draw(MODEL_SPHERE, showNormals, true, &viewMat, &modelviewMat);
        scene_popMatrix();
    }

    debug_popRenderScope();
}