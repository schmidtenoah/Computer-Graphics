/**
 * @file rendering.c
 * @brief Implementation of rendering and visual effects.
 *
 * Manages entire rendering system including:
 * - Dynamic curve evaluation from control points
 * - Interactive draggable control point buttons
 * - Animated game objects (paper airplane with shadow, rotating stars, drifting clouds)
 * - Gradient background using fullscreen quad
 * - Debug visualization (collision circles, convex hull, control polygon, normals)
 * - Viewport
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#include "rendering.h"
#include "input.h"
#include "model.h"
#include "shader.h"
#include "utils.h"
#include "logic.h"

/** Button management*/
#define BUTTON_DETECTION_RANGE 0.8f
#define BUTTON_DRAG_EDGE_DISTANCE 0.05f
#define START_BUTTON_EDGE_DIST 0.8f
#define END_BUTTON_EDGE_DIST START_BUTTON_EDGE_DIST
#define BUTTON_RADIUS 0.05f

/** Projection data*/
#define BOUNDS 1
#define NEAR_PLANE 0.01f
#define FAR_PLANE 2.0f
#define VEC3(x, y, z) ((vec3){(float) x, (float) y, (float) z})

/** Colors*/
#define BUTTON_NORMAL_COLOR VEC3(1, 1, 1)
#define BUTTON_HOVER_COLOR VEC3(1, 0, 0)
#define BUTTON_SELECTED_COLOR VEC3(1, 0, 1)
#define BUTTON_DISABLED_COLOR VEC3(0, 0, 0)
#define COLLIDER_COLOR VEC3(1, 0.5, 0)
#define AIRPLANE_COLOR VEC3(1, 1, 1)
#define STAR_COLOR VEC3(1, 1, 0)

/** Star rotation speed*/
#define STAR_ROTATION_SPEED 0.2f

/** Cloud Sizes*/
#define CLOUD_CENTER_SIZE 1.0f
#define CLOUD_SIDE_SIZE 0.85f
#define CLOUD_BOTTOM_SIZE 0.7f
#define CLOUD_TOP_SIZE 0.75f
#define CLOUD_FAR_SIZE 0.6f

////////////////////////    LOCAL    ////////////////////////////

/** Global rendering data (viewport, projection bounds, screen resolution) */
static RenderingData g_renderingData;

/** Pointer to the button currently being dragged (NULL if none) */
static Circle *g_draggedButton = NULL;

/** Flag if buttons have been initialized */
static bool g_buttonInitialized = false;

/** Array of pointers to button circles */
static Circle *g_buttons[BUTTON_COUNT];

/** Storage array for all button data */
static Circle g_buttonStorage[BUTTON_COUNT];

/** VAO and VBO for background gradient quad*/
static GLuint g_backgroundVAO = 0, g_backgroundVBO = 0;

/**
 * Curve rendering data.
 */
static struct {
    vec2 vertices[CURVE_MAX_VERTICES];
    vec3 normalVertices[CURVE_MAX_VERTICES];
    int numVertices;
} curve;

/**
 * Handles mouse interaction with control point buttons.
 *
 * Detects button clicks/drags, updates button positions during dragging,
 * and draws all buttons with colors based on state:
 * - Black: disabled (first/last button or during flight)
 * - Magenta: currently being dragged
 * - Red: hovered
 * - White: normal state
 *
 * First and last buttons are fixed and cannot be dragged.
 * Button dragging is clamped to stay within screen bounds.
 *
 * @param data Pointer to InputData containing mouse state and settings
 */
static void checkAndDrawButtons(InputData *data) {
    bool foundSelected = false;

    debug_pushRenderScope("Buttons");
    scene_pushMatrix();

    int btnCnt = data->curve.buttonCount;

    // Loop through all buttons
    for (int i = 0; i < btnCnt; ++i) {
        scene_pushMatrix();
        Circle *btn = g_buttons[i];

        // Handle disabled buttons
        if (i == 0 || i == btnCnt - 1 || data->game.isFlying) {
            shader_setColor(BUTTON_DISABLED_COLOR);
            scene_translate(btn->center[0], btn->center[1], 0);
            scene_scale(btn->r, btn->r, 1);
            model_draw(MODEL_CIRCLE);

            scene_popMatrix();
            continue;
        }

        // Check if mouse over this button
        bool isInside = utils_isMouseInCircle(data->mouse.xPos, data->mouse.yPos, btn, &g_renderingData, BUTTON_DETECTION_RANGE);

        // Handle mouse press / dragging
        if (data->mouse.action == GLFW_PRESS && isInside && !g_draggedButton) {
            g_draggedButton = btn;
            foundSelected = true;
        }

        // Handle mouse stop drag
        if (data->mouse.action == GLFW_RELEASE && g_draggedButton == btn) {
            g_draggedButton = NULL;
        }

        // Button colors
        if (!foundSelected && isInside) {
            foundSelected = true;
            if (data->mouse.button == GLFW_MOUSE_BUTTON_LEFT && data->mouse.action == GLFW_PRESS && g_draggedButton == btn) {
                shader_setColor(BUTTON_SELECTED_COLOR);
            } else if (g_draggedButton == NULL) {
                shader_setColor(BUTTON_HOVER_COLOR);
            } else {
                shader_setColor(BUTTON_NORMAL_COLOR);
            }
        } else {
            shader_setColor(BUTTON_NORMAL_COLOR);
        }

        // update drag position
        if (g_draggedButton == btn && data->mouse.action != GLFW_RELEASE) {

            // Convert mouse screen coord to world
            float mouseX_ndc = (float)data->mouse.xPos / g_renderingData.screenRes[0];
            float mouseY_ndc = 1.0f - (float)data->mouse.yPos / g_renderingData.screenRes[1];
            float sceneX = g_renderingData.left + mouseX_ndc * (g_renderingData.right - g_renderingData.left);
            float sceneY = g_renderingData.bottom + mouseY_ndc * (g_renderingData.top - g_renderingData.bottom);

            // Clamp button pos
            float d = BUTTON_DRAG_EDGE_DISTANCE;
            btn->center[0] = glm_clamp(sceneX, g_renderingData.left + d, g_renderingData.right - d);
            btn->center[1] = glm_clamp(sceneY, g_renderingData.bottom + d, g_renderingData.top - d);

            // Mark curve for recalc
            data->curve.buttonsChanged = true;
            shader_setColor(BUTTON_SELECTED_COLOR);
        }

        // Render button
        scene_translate(btn->center[0], btn->center[1], 0);
        scene_scale(btn->r, btn->r, 1);
        model_draw(MODEL_CIRCLE);

        scene_popMatrix();
    }

    scene_popMatrix();
    debug_popRenderScope();
}

/**
 * Renders the control polygon connecting all control points.
 * Draws lines between consecutive control points in cyan.
 *
 * @note Only called if InputData.curve.drawPolygon is enabled.
 *
 * @param ctrl 2D Vec of control point positions
 * @param n Number of control points
 */
static void drawControlPolygon(vec2 *ctrl, int n) {
    scene_pushMatrix();

    model_updateCurve(ctrl, NULL, n);
    shader_setColor(VEC3(0,1,1));
    model_drawCurve(n, 2.0f);

    scene_popMatrix();
}

/**
 * Evaluates and renders the curve (spline or bezier).
 *
 * Only recalculates curve vertices if resolution or control points have changed.
 * Samples the curve function in steps from t=0.0 to t=1.0,
 * calculates normal vectors and draws as a red line strip.
 *
 * @param data Pointer to InputData containing curve settings and flags
 * @param ctrl 2D vector of control point positions
 * @param step Resolution step size for curve sampling
 * @param width Line width in pixels for rendering
 * @param n Number of control points
 */
static void drawCurve(InputData *data, vec2 *ctrl, float step, float width, int n) {
    scene_pushMatrix();

    // Recalc curve vertices if needed
    if (data->curve.resolutionChanged || data->curve.buttonsChanged) {

        // Reset vertex count and sample curve
        curve.numVertices = 0;
        for (float T = 0.0f; T <= 1.0f && curve.numVertices < CURVE_MAX_VERTICES; T += step) {
            // Eval curve at T!
            data->curve.curveEval(ctrl, n, T, curve.vertices[curve.numVertices], &data->curve.buttonsChanged);
            curve.numVertices++;
        }

        // always interpolate last step
        data->curve.curveEval(ctrl, n, 1.0f, curve.vertices[curve.numVertices - 1], &data->curve.buttonsChanged);

        utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);

        data->curve.resolutionChanged = false;
    }

    model_updateCurve(curve.vertices, curve.normalVertices, curve.numVertices);
    if (data->curve.buttonsChanged) data->curve.buttonsChanged = false;
    shader_setColor(VEC3(1, 0, 0));
    model_drawCurve(curve.numVertices, width);

    scene_popMatrix();
}

/**
 * Renders the convex hull of control point buttons.
 *
 * Takes smallest convex polygon containing all button positions
 * and draws it as a green line loop.
 *
 * @note Only called if InputData.curve.drawConvexHull is enabled.
 *
 * @param btnCnt Number of control point buttons
 */
static void drawConvexHull(int btnCnt) {
    vec2 points[BUTTON_COUNT];
    for (int i = 0; i < btnCnt; ++i) {
        glm_vec2_copy(g_buttons[i]->center, points[i]);
    }

    vec2 hull[BUTTON_COUNT + 1];
    int hullCount = utils_convexHullVec2(points, hull, btnCnt);

    model_updateCurve(hull, NULL, hullCount);
    shader_setColor(VEC3(0,1,0));
    model_drawCurve(hullCount, 2.0f);
}

/**
 * Renders animated cloud obstacles.
 *
 * Each cloud consists of 8 overlapping circles of varying sizes to create
 * a fluffy appearance. Clouds slowly drift horizontally using sine.
 * Collision circles are drawn if showColliders is enabled.
 *
 * Animation pauses when game is paused.
 *
 * @param data Pointer to InputData containing cloud positions and settings
 */
static void drawClouds(InputData *data) {
    scene_pushMatrix();

    vec2 pos;

    // Loop through all clouds
    for (int i = 0; i < data->game.clouds.n; i++) {
        glm_vec2_copy(data->game.clouds.pos[i], pos);

        // Soft drift animation
        float drift = 0.015f * sinf((float)glfwGetTime() * 0.3f + i * 1.5f);
        if (!data->paused) {
            pos[0] += drift;
        }

        // Cloud composition
        vec2 offsets[] = {
            { 0.0f, 0.0f },      // Center
            { -0.04f, 0.01f },   // Left
            { 0.04f, 0.01f },    // Right
            { -0.02f, -0.02f },  // Bottom left
            { 0.02f, -0.02f },   // Bottom right
            { 0.0f, 0.025f },    // Top
            { -0.06f, 0.0f },    // Far left
            { 0.06f, 0.0f },     // Far right
        };

        float sizes[] = {
            CLOUD_CENTER_SIZE,   // Center
            CLOUD_SIDE_SIZE,     // Left
            CLOUD_SIDE_SIZE,     // Right
            CLOUD_BOTTOM_SIZE,   // Bottom left
            CLOUD_BOTTOM_SIZE,   // Bottom right
            CLOUD_TOP_SIZE,      // Top
            CLOUD_FAR_SIZE,      // Far left
            CLOUD_FAR_SIZE,      // Far right
        };

        // Draw cloud parts from back to front for proper layering
        for (int j = 7; j >= 0; j--) {
            scene_pushMatrix();

            float baseSize = data->game.clouds.colliderRadius * 0.9f;
            float scale = baseSize * sizes[j];

            scene_translate(pos[0] + offsets[j][0], pos[1] + offsets[j][1], 0.0f);
            scene_scale(scale, scale * 0.85f, 1.0f);

            // Gradient from white center to gray edges
            float brightness = 0.85f + 0.15f * sizes[j];
            shader_setColor((vec3){brightness, brightness, brightness * 1.05f});

            model_draw(MODEL_CIRCLE);
            scene_popMatrix();
        }

        // Collider visualization
        if (data->game.showColliders) {
            scene_pushMatrix();
            scene_translate(data->game.clouds.pos[i][0], data->game.clouds.pos[i][1], 0.0f);
            scene_scale(data->game.clouds.colliderRadius, data->game.clouds.colliderRadius, 1.0f);
            shader_setColor(COLLIDER_COLOR);
            model_draw(MODEL_CIRCLE);
            scene_popMatrix();
        }
    }

    scene_popMatrix();
}

/**
 * Renders collectible stars with rotation animation.
 *
 * Draws stars with continuous rotation based on game time.
 *
 * Collision circles are drawn if showColliders is enabled.
 * Animation pauses when game is paused.
 *
 * @param data Pointer to InputData containing star positions and collection state
 */
static void drawStars(InputData *data) {
    scene_pushMatrix();

    vec2 pos;
    for (int i = 0; i < data->game.stars.n; i++) {
        if (data->game.collected[i]) {
            continue;
        }

        glm_vec2_copy(data->game.stars.pos[i], pos);

        float angleOffset = (float) i * 10.3f;
        float rotationAngle = (float) glfwGetTime() * STAR_ROTATION_SPEED + angleOffset;
        if (data->paused) {
            rotationAngle = 0;
        }

        // star
        scene_pushMatrix();
        scene_translate(pos[0], pos[1], 0.0f);
        scene_rotate(rotationAngle * 180.0f / (float)M_PI, 0.0f, 0.0f, 1.0f);
        scene_scale(data->game.stars.colliderRadius * 2.0f, data->game.stars.colliderRadius * 2.0f, 1.0f);
        shader_setColor(STAR_COLOR);
        model_draw(MODEL_STAR);
        scene_popMatrix();

        // collider
        if (data->game.showColliders) {
            scene_pushMatrix();
            scene_translate(pos[0], pos[1], 0.0f);
            scene_scale(data->game.stars.colliderRadius, data->game.stars.colliderRadius, 1.0f);
            shader_setColor(COLLIDER_COLOR);
            model_draw(MODEL_CIRCLE);
            scene_popMatrix();
        }
    }

    scene_popMatrix();
}

/**
 * Renders the paper airplane with shadow and fold effect.
 *
 * Draws three layers:
 * 1. Shadow (dark gray, offset below and right)
 * 2. Main body (white triangle)
 * 3. Center fold (darker triangle overlay for paper fold effect)
 *
 * Airplane is positioned and rotated according to game state (curve).
 * Collision circles for the three triangle vertices are drawn if showColliders is enabled.
 *
 * @param data Pointer to InputData containing airplane position, rotation
 */
static void drawAirplane(InputData *data) {
    scene_pushMatrix();

    // 1. Shadow (drawn first so it's behind)
    scene_pushMatrix();
    shader_setColor((vec3){0.3f, 0.3f, 0.4f}); // Dark shadow
    scene_translate(data->game.airplane.position[0] + 0.015f,
                   data->game.airplane.position[1] - 0.015f, 0.0f);
    scene_rotate(data->game.airplane.rotation * 180.0f / (float)M_PI, 0, 0, 1);
    scene_scale(0.16f, 0.21f, 1.0f);
    model_draw(MODEL_TRIANGLE);
    scene_popMatrix();

    // 2. Main white body
    scene_pushMatrix();
    shader_setColor((vec3){0.95f, 0.95f, 1.0f});
    scene_translate(data->game.airplane.position[0], data->game.airplane.position[1], 0.0f);
    scene_rotate(data->game.airplane.rotation * 180.0f / (float)M_PI, 0, 0, 1);
    scene_scale(0.15f, 0.2f, 1.0f);
    model_draw(MODEL_TRIANGLE);

    // 3. Center fold (darker triangle on top for paper fold effect)
    scene_pushMatrix();
    scene_scale(0.3f, 0.95f, 1.0f);  // Thinner triangle in the center
    shader_setColor((vec3){0.85f, 0.85f, 0.9f}); // Slightly darker for fold-looking
    model_draw(MODEL_TRIANGLE);
    scene_popMatrix();
    scene_popMatrix();
    scene_popMatrix();

    // Colliders
    if (data->game.showColliders) {
        float colliderRadius = data->game.airplane.colliderRadius;
        shader_setColor(COLLIDER_COLOR);

        vec2 v;
        for (int i = 0; i < 3; i++) {
            glm_vec2_copy(data->game.airplane.vertices[i], v);
            scene_pushMatrix();
            scene_translate(v[0], v[1], 0.0f);
            scene_scale(colliderRadius, colliderRadius, 1.0f);
            model_draw(MODEL_CIRCLE);
            scene_popMatrix();
        }
    }
}

/**
 * Initializes curve vertices by evaluation function (spline / bezier).
 *
 * Takes control points from button positions, samples the curve at steps
 * and calculates normal vectors.
 */
static void initCurve(void) {
    InputData *input = getInputData();
    int btnCnt = input->curve.buttonCount;
    vec2 ctrl[BUTTON_COUNT];
    for (int k = 0; k < btnCnt; ++k) {
        ctrl[k][0] = g_buttons[k]->center[0];
        ctrl[k][1] = g_buttons[k]->center[1];
    }

    for (float T = 0.0f; T <= 1.0f && curve.numVertices < CURVE_MAX_VERTICES; T += input->curve.resolution) {
        input->curve.curveEval(ctrl, btnCnt, T, curve.vertices[curve.numVertices], &input->curve.buttonsChanged);
        curve.numVertices++;
    }

    input->curve.curveEval(ctrl, btnCnt, 1.0f, curve.vertices[curve.numVertices - 1], &input->curve.buttonsChanged);
    utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);
}

/**
 * Renders the vertical gradient background.
 *
 * Draws a fullscreen quad with gradient shader to create sky-like effect.
 * Depth testing is disabled to ensure background is always behind other objects.
 */
static void drawGradientBackground(void) {
    glDisable(GL_DEPTH_TEST);
    shader_renderGradient();
    glBindVertexArray(g_backgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

/**
 * Updates positions of first and last control point buttons after window resize.
 *
 * Repositions edge buttons to maintain fixed distance from screen edges
 * based on START_BUTTON_EDGE_DIST and END_BUTTON_EDGE_DIST constants.
 *
 * @note Middle buttons are not affected.
 *
 * @param btnCnt Total number of control point buttons
 */
static void updateEdgeButtons(int btnCnt) {
    if (g_buttons == NULL || g_buttons[0] == NULL) {
        return;
    }

    // First button -> right edge
    g_buttons[0]->center[0] = g_renderingData.left * START_BUTTON_EDGE_DIST;
    g_buttons[0]->center[1] = 0.0f;

    // Last button -> left edge
    g_buttons[btnCnt-1]->center[0] = g_renderingData.right * END_BUTTON_EDGE_DIST;
    g_buttons[btnCnt-1]->center[1] = 0.0f;
}

////////////////////////    LOCAL    ////////////////////////////

void initButtons(int btnCnt) { 
    for (int i = 0; i < btnCnt; i++) { 
        g_buttons[i] = &g_buttonStorage[i];
        g_buttons[i]->r = BUTTON_RADIUS;
        if (i == 0) { 
            g_buttons[i]->center[0] = g_renderingData.left * START_BUTTON_EDGE_DIST;
            g_buttons[i]->center[1] = 0.0f;
        } else if (i == btnCnt - 1) { 
            g_buttons[i]->center[0] = g_renderingData.right * END_BUTTON_EDGE_DIST;
            g_buttons[i]->center[1] = 0.0f;
        } else { 
            float t = (float)i / (btnCnt- 1); 
            g_buttons[i]->center[0] = g_renderingData.left * START_BUTTON_EDGE_DIST * (1.0f - t)
                                  + g_renderingData.right * END_BUTTON_EDGE_DIST * t;
            g_buttons[i]->center[1] = 0.0f;
        } 
    } 
    g_buttonInitialized = true;
}

void rendering_init(void) {
    memset(&g_renderingData, 0, sizeof(RenderingData));

    scene_lookAt((vec3) {0, 0, 1}, GLM_VEC3_ZERO, GLM_YUP);

    // Gradient Start
    float backgroundVertices[] = {
        // Positions
        -1.0f, -1.0f, 0.0f,  // Links unten
         1.0f, -1.0f, 0.0f,  // Rechts unten
        -1.0f,  1.0f, 0.0f,  // Links oben

         1.0f, -1.0f, 0.0f,  // Rechts unten
         1.0f,  1.0f, 0.0f,  // Rechts oben
        -1.0f,  1.0f, 0.0f   // Links oben
    };

    glGenVertexArrays(1, &g_backgroundVAO);
    glGenBuffers(1, &g_backgroundVBO);

    glBindVertexArray(g_backgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backgroundVertices),
                 backgroundVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0); // Gradient end

    // OpenGL Flags
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_DEPTH_TEST);

    initCurve();
    shader_load();
}

void rendering_draw(void) {
    InputData* input = getInputData();

    drawGradientBackground();

    if(input->showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
    glDisable(GL_DEPTH_TEST);

    debug_pushRenderScope("Scene");
    scene_pushMatrix();

    checkAndDrawButtons(input);

    int btnCnt = input->curve.buttonCount;
    vec2 ctrl[BUTTON_COUNT];
    for (int k = 0; k < btnCnt; ++k) {
        ctrl[k][0] = g_buttons[k]->center[0];
        ctrl[k][1] = g_buttons[k]->center[1];
    }

    if (input->curve.drawPolygon) {
        drawControlPolygon(ctrl, btnCnt);
    }
    if (input->curve.drawConvexHull) {
        drawConvexHull(btnCnt);
    }

    drawCurve(input, ctrl, input->curve.resolution, input->curve.width, btnCnt);
    logic_update(input, ctrl, btnCnt);

    drawClouds(input);
    drawStars(input);
    drawAirplane(input);

    scene_popMatrix();
    debug_popRenderScope();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void rendering_cleanup(void) {
    if (g_backgroundVBO) {
        glDeleteBuffers(1, &g_backgroundVBO);
    }
    if (g_backgroundVAO) {
        glDeleteVertexArrays(1, &g_backgroundVAO);
    }

    shader_cleanup();
}

void rendering_resize(int width, int height, int btnCnt) {
    g_renderingData.screenRes[0] = width;
    g_renderingData.screenRes[1] = height;

    g_renderingData.aspect = (float) width / height;

    g_renderingData.left   = -BOUNDS * (g_renderingData.aspect >= 1 ? g_renderingData.aspect : 1);
    g_renderingData.right  =  BOUNDS * (g_renderingData.aspect >= 1 ? g_renderingData.aspect : 1);
    g_renderingData.bottom = -BOUNDS / (g_renderingData.aspect < 1  ? g_renderingData.aspect : 1);
    g_renderingData.top    =  BOUNDS / (g_renderingData.aspect < 1  ? g_renderingData.aspect : 1);

    scene_ortho(
        g_renderingData.left, g_renderingData.right, g_renderingData.bottom, g_renderingData.top,
        NEAR_PLANE, FAR_PLANE
    );

    if (g_buttonInitialized) {
        updateEdgeButtons(btnCnt);
    } else {
        initButtons(btnCnt);
    }
}
