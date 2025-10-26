/**
 * @file logic.c
 * @brief Implementation of game logic, physics and level management.
 *
 * Manages all game mechanics including:
 * - Airplane movement along the curve with speed along curve slope.
 * - Collision detection (stars, clouds, airplane vertices)
 * - Level progression and win/lose conditions
 * - Six pre-defined levels with increasing difficulty
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "logic.h"
#include "input.h"
#include "model.h"
#include "utils.h"

/** Game constants*/
#define AIRPLANE_START_T 0.0f
#define AIRPLANE_COLLIDER_RADIUS 0.03f
#define AIRPLANE_DEFAULT_SPEED 0.2f
#define CLOUD_COLLISION_RADIUS 0.08f
#define STAR_COLLISION_RADIUS 0.05f

#define LEVEL_COUNT 6

/**
 * Level data.
 * Defines all properties for a level including:
 * star positions, cloud positions, collision radii and control point count.
 */
typedef struct {
    vec2 *stars;
    int starCount;
    float starRadius;
    vec2 *clouds;
    int cloudCount;
    float cloudRadius;
    int buttonCount;
} Level;

////////////////////////    LOCAL    ////////////////////////////

// Level 1
static vec2 starsLevel1[]  = { {0.0f, -0.4f} };
static vec2 cloudsLevel1[] = { 0 }; // none

// Level 2
static vec2 starsLevel2[]  = { {-0.6f, -0.2f}, {0.2f, 0.2f}, {0.6f, -0.4f} };
static vec2 cloudsLevel2[] = { { 0.1f, -0.2f } };

// Level 3
static vec2 starsLevel3[]  = { {-1.0f, 0.7f}, {0.0f, 0.8f} };
static vec2 cloudsLevel3[] = { {-0.6f, 0.5f}, {0.8f, 0.6f} };

// Level 4
static vec2 starsLevel4[]  = { {-0.7f, -0.4f}, {-0.3f, 0.2f}, {0.3f, -0.2f}, {0.7f, -0.6f} };
static vec2 cloudsLevel4[] = { {-0.4f, 0.5f}, {0.2f, 0.7f}, {0.8f, 0.2f} };

// Level 5
static vec2 starsLevel5[]  = { {-0.8f, -0.6f}, {-0.4f, 0.0f}, {0.0f, 0.6f}, {0.4f, -0.2f}, {0.8f, 0.4f} };
static vec2 cloudsLevel5[] = { {-0.6f, 0.4f}, {-0.2f, 0.8f}, {0.2f, 0.7f}, {0.6f, 0.8f} };

// Level 6
static vec2 starsLevel6[] = {
    {-0.9f,-0.8f}, {-0.8f,-0.6f}, {-0.7f,-0.7f}, {-0.6f,-0.4f}, {-0.5f,-0.2f},
    {-0.4f, 0.0f}, {-0.3f, 0.2f}, {-0.2f, 0.4f}, {-0.1f, 0.6f}, { 0.0f, 0.8f},
    { 0.1f, 0.6f}, { 0.2f, 0.4f}, { 0.3f, 0.2f}, { 0.4f, 0.0f}, { 0.5f,-0.2f},
    { 0.6f,-0.4f}, { 0.7f,-0.6f}, { 0.8f,-0.8f}, { 0.9f,-0.5f}, {-0.9f, 0.5f},
    {-0.7f, 0.7f}, {-0.5f, 0.8f}, {-0.3f, 0.9f}, {-0.1f,-0.9f}, { 0.1f,-0.7f},
    { 0.3f,-0.9f}, { 0.5f, 0.9f}, { 0.7f, 0.5f}, { 0.9f, 0.1f}, {-0.8f, 0.1f},
    {-0.6f, 0.3f}, {-0.4f, 0.5f}, {-0.2f,-0.5f}, { 0.0f,-0.3f}, { 0.2f,-0.1f},
    { 0.4f, 0.1f}, { 0.6f, 0.3f}, { 0.8f, 0.5f}, { 0.9f,-0.3f}, {-0.9f,-0.1f}
};
static vec2 cloudsLevel6[] = { {-0.1f, 0.1f} };

/**
 * Array of all levels
 */
static Level levels[LEVEL_COUNT] = {
    { starsLevel1, 1, STAR_COLLISION_RADIUS, cloudsLevel1, 0, CLOUD_COLLISION_RADIUS, 4 },
    { starsLevel2, 3, STAR_COLLISION_RADIUS, cloudsLevel2, 1, CLOUD_COLLISION_RADIUS, 5 },
    { starsLevel3, 2, STAR_COLLISION_RADIUS * 1.7f, cloudsLevel3, 2, CLOUD_COLLISION_RADIUS * 2.5f, 6 },
    { starsLevel4, 4, STAR_COLLISION_RADIUS, cloudsLevel4, 3, CLOUD_COLLISION_RADIUS, 8 },
    { starsLevel5, 5, STAR_COLLISION_RADIUS, cloudsLevel5, 4, CLOUD_COLLISION_RADIUS, 10 },
    { starsLevel6, 40, STAR_COLLISION_RADIUS, cloudsLevel6, 1, CLOUD_COLLISION_RADIUS, 20 } 
};

/** Current position of airplane along curve [0.0, 1.0] */
static float g_curveT = AIRPLANE_START_T;

/** Current level */
static int g_currLevel = 0;

/**
 * Copies level data into InputData structure.
 * Updates star positions, cloud positions, collision radii, collection flags
 * and control point count.
 *
 * Sets curve to spline mode for levels with more than 4 points.
 *
 * @param data Pointer to InputData
 * @param level Pointer to Level configuration to copy from
 */
static void setLevelData(InputData *data, Level *level) {
    // Star data
    data->game.stars.pos = level->stars;
    data->game.stars.n = level->starCount;
    data->game.stars.colliderRadius = level->starRadius;

    // Cloud data
    data->game.clouds.pos = level->clouds;
    data->game.clouds.n = level->cloudCount;
    data->game.clouds.colliderRadius = level->cloudRadius;

    // Reset star collection flags
    for (int i = 0; i < level->starCount; ++i) {
        data->game.collected[i] = false;
    }

    // Curvev configs
    data->curve.buttonCount = level->buttonCount;
    if (level->buttonCount != 4) {
        data->curve.curveEval = utils_evalSpline;
    }
    data->curve.buttonsChanged = true;
}

/**
 * Goes to the next level.
 * Updates level data and puts according control point buttons.
 *
 * @param data Pointer to InputData to update
 */
static void loadNextLevel(InputData *data) {
    g_currLevel = (g_currLevel + 1) % LEVEL_COUNT;
    setLevelData(data, &levels[g_currLevel]);
    data->game.currentLevel = g_currLevel;
    initButtons(levels[g_currLevel].buttonCount);
}

/**
 * Reloads the current level configuration.
 * Resets star collection flags and control point count without changing level.
 * Used when player fails (cloud collision) or manually restarts.
 *
 * @param data Pointer to InputData to update
 */
static void reloadLevel(InputData *data) {
    setLevelData(data, &levels[g_currLevel]);
}

/**
 * Checks for collision between airplane and cloud obstacle.
 * Tests all three airplane vertices against all clouds using circle-circle collision.
 *
 * @param data Pointer to InputData containing airplane and cloud positions
 * @return true if any airplane vertex collides with any cloud, false otherwise
 */
static bool checkCloudCollision(InputData *data) {
    for (int c = 0; c < data->game.clouds.n; ++c) {
        for (int i = 0; i < 3; ++i) {
            if (utils_circleInCircle(data->game.airplane.vertices[i], data->game.airplane.colliderRadius,
                               data->game.clouds.pos[c], data->game.clouds.colliderRadius)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Checks win condition: all stars collected.
 * If all stars collected, advances to next level.
 * If any stars missed, reloads current level.
 * Called when airplane reaches end of curve (T >= 1.0).
 *
 * @param data Pointer to InputData containing star collection state
 */
static void checkWin(InputData *data) {
    bool allCollected = true;
    for (int i = 0; i < data->game.stars.n; ++i) {
        if (!data->game.collected[i]) {
            allCollected = false;
            break;
        }
    }

    if (allCollected) {
        loadNextLevel(data);
    } else {
        reloadLevel(data);
    }
}

/**
 * Updates airplane position, rotation and collision geometry each frame.
 *
 * Physics simulation:
 * 1. Calculate tangent at current curve position for rotation
 * 2. Apply slope-dependent speed (faster downhill, slower uphill)
 * 3. Advance curve parameter based on speed and delta time
 * 4. Evaluate curve position and offset airplane perpendicular to tangent
 * 5. Calculate rotation angle from tangent direction
 * 7. Check for cloud collision and reset if hit
 *
 * @param data Pointer to InputData containing airplane and game state
 * @param ctrl Array of control points defining the curve
 * @param n Number of control points
 */
static void airplaneUpdate(InputData *data, vec2 *ctrl, int n) {
    vec2 T;

    // Calc tangent
    utils_getTangent(data->curve.curveEval, ctrl, n, g_curveT, T);
    glm_vec2_normalize(T);

    // slope-dependent speed
    if (data->game.isFlying) {
        float slopeInfluence = 1.3f;
        float slopeFactor = 1.0f - slopeInfluence * T[1];
        slopeFactor = glm_clamp(slopeFactor, 0.5f, 5.0f);
        g_curveT += data->deltaTime * data->game.airplane.defaultSpeed * slopeFactor;
        if (g_curveT >= 1.0f) {
            g_curveT = AIRPLANE_START_T;
            data->game.isFlying = false;
            checkWin(data);
        }
    } else {
        g_curveT = AIRPLANE_START_T;
    }

    // position on spline
    vec2 P;
    data->curve.curveEval(ctrl, n, g_curveT, P, false);

    // rotation (tip points along tangent)
    float angle = atan2f(T[1], T[0]) - (float)M_PI_2;

    vec2 offsetDir = { -T[1], T[0] };
    float offset = 0.05f;
    vec2 Poffset = { P[0] + offsetDir[0]*offset, P[1] + offsetDir[1]*offset };

    // Update airplane pos and rot
    data->game.airplane.position[0] = Poffset[0];
    data->game.airplane.position[1] = Poffset[1];
    data->game.airplane.rotation = angle;

    vec2 local[3] = { {0, 0.16f}, {-0.08f, -0.08f}, {0.08f, -0.08f} };
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    for (int i = 0; i < 3; ++i) {
        data->game.airplane.vertices[i][0] = cosA * local[i][0] - sinA * local[i][1] + Poffset[0];
        data->game.airplane.vertices[i][1] = sinA * local[i][0] + cosA * local[i][1] + Poffset[1];
    }

    if (data->game.isFlying && checkCloudCollision(data)) {
        g_curveT = AIRPLANE_START_T;
        data->game.isFlying = false;
        reloadLevel(data);
    }
}

/**
 * Checks for collision between airplane and collectible stars.
 * Tests all three airplane vertices against all uncollected stars.
 * Marks stars as collected when hit. Only checks during flight.
 *
 * @param data Pointer to InputData containing airplane and star positions
 */
static void checkStarCollision(InputData *data) {
    if (!data->game.isFlying) {
        return;
    }

     for (int s = 0; s < data->game.stars.n; ++s) {
        if (data->game.collected[s]) continue;

        for (int i = 0; i < 3; ++i) {
            if (utils_circleInCircle(data->game.airplane.vertices[i], data->game.airplane.colliderRadius,
                               data->game.stars.pos[s], data->game.stars.colliderRadius)) {
                data->game.collected[s] = true;
                break;
            }
        }
    }
}

////////////////////////    PUBLIC    ////////////////////////////

void logic_update(InputData *data, vec2 *ctrl, int n) {
    airplaneUpdate(data, ctrl, n);
    checkStarCollision(data);
    checkCloudCollision(data);
}

void logic_init() {
    InputData *data = getInputData();
    data->game.airplane.colliderRadius = AIRPLANE_COLLIDER_RADIUS;
    data->game.airplane.defaultSpeed = AIRPLANE_DEFAULT_SPEED;
    reloadLevel(data);
    initButtons(levels[g_currLevel].buttonCount);
}

void logic_skipLevel(InputData *data) {
    data->game.isFlying = false;
    g_curveT = AIRPLANE_START_T;
    loadNextLevel(data);
}

void logic_restartLevel(InputData *data) {
    data->game.isFlying = false;
    g_curveT = AIRPLANE_START_T;
    reloadLevel(data);
}

void loadLevel(int idx, InputData *data) {
    data->game.isFlying = false;
    g_curveT = AIRPLANE_START_T;
    g_currLevel = idx;
    setLevelData(data, &levels[g_currLevel]);
    data->game.currentLevel = g_currLevel;
    initButtons(levels[g_currLevel].buttonCount);
}
