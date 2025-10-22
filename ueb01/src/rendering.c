#include "rendering.h"
#include "input.h"
#include "model.h"
#include "shader.h"
#include "utils.h"
#include "logic.h"

#define BUTTON_DETECTION_RANGE 0.8f
#define BUTTON_DRAG_EDGE_DISTANCE 0.05f
#define START_BUTTON_EDGE_DIST 0.8f
#define END_BUTTON_EDGE_DIST START_BUTTON_EDGE_DIST
#define BUTTON_RADIUS 0.05f

#define BOUNDS 1
#define NEAR_PLANE 0.01f
#define FAR_PLANE 2.0f

#define VEC3(x, y, z) ((vec3){(float) x, (float) y, (float) z})

#define BUTTON_NORMAL_COLOR VEC3(1, 1, 1)
#define BUTTON_HOVER_COLOR VEC3(1, 0, 0)
#define BUTTON_SELECTED_COLOR VEC3(1, 0, 1)
#define BUTTON_DISABLED_COLOR VEC3(0, 0, 0)

#define COLLIDER_COLOR VEC3(1, 0.5, 0)
#define AIRPLANE_COLOR VEC3(1, 1, 1)
#define STAR_COLOR VEC3(1, 1, 0)
#define STAR_ROTATION_SPEED 0.2f


// LOCAL

static RenderingData rd;
static Circle *draggedButton = NULL;

static bool buttonInitilized = false;
static Circle *buttons[BUTTON_COUNT];
static Circle buttonStorage[BUTTON_COUNT];

static GLuint backgroundVAO = 0, backgroundVBO = 0;

static struct {
    vec2 vertices[CURVE_MAX_VERTICES];
    vec3 normalVertices[CURVE_MAX_VERTICES];
    int numVertices;
} curve;

static void checkAndDrawButtons(InputData *data) {
    bool foundSelected = false;

    debug_pushRenderScope("Buttons");
    scene_pushMatrix();

    int btnCnt = data->curve.buttonCount;
    for (int i = 0; i < btnCnt; ++i) {
        scene_pushMatrix();
        Circle *btn = buttons[i];

        if (i == 0 || i == btnCnt - 1 || data->game.isFlying) {
            shader_setColor(BUTTON_DISABLED_COLOR);
            scene_translate(btn->center[0], btn->center[1], 0);
            scene_scale(btn->r, btn->r, 1);
            model_draw(MODEL_CIRCLE);

            scene_popMatrix();
            continue;
        }

        bool isInside = utils_isMouseInCircle(data->mouse.xPos, data->mouse.yPos, btn, &rd, BUTTON_DETECTION_RANGE);

        if (data->mouse.action == GLFW_PRESS && isInside && !draggedButton) {
            draggedButton = btn;
            foundSelected = true;
        }
        if (data->mouse.action == GLFW_RELEASE && draggedButton == btn) {
            draggedButton = NULL;
        }

        if (!foundSelected && isInside) {
            foundSelected = true;
            if (data->mouse.button == GLFW_MOUSE_BUTTON_LEFT && data->mouse.action == GLFW_PRESS && draggedButton == btn) {
                shader_setColor(BUTTON_SELECTED_COLOR);
            } else if (draggedButton == NULL) {
                shader_setColor(BUTTON_HOVER_COLOR);
            } else {
                shader_setColor(BUTTON_NORMAL_COLOR);
            }
        } else {
            shader_setColor(BUTTON_NORMAL_COLOR);
        }

        // update drag position
        if (draggedButton == btn && data->mouse.action != GLFW_RELEASE) {
            float mouseX_ndc = (float)data->mouse.xPos / rd.screenRes[0];
            float mouseY_ndc = 1.0f - (float)data->mouse.yPos / rd.screenRes[1];
            float sceneX = rd.left + mouseX_ndc * (rd.right - rd.left);
            float sceneY = rd.bottom + mouseY_ndc * (rd.top - rd.bottom);

            float d = BUTTON_DRAG_EDGE_DISTANCE;
            btn->center[0] = glm_clamp(sceneX, rd.left + d, rd.right - d);
            btn->center[1] = glm_clamp(sceneY, rd.bottom + d, rd.top - d);

            data->curve.buttonsChanged = true;
            shader_setColor(BUTTON_SELECTED_COLOR);
        }

        scene_translate(btn->center[0], btn->center[1], 0);
        scene_scale(btn->r, btn->r, 1);
        model_draw(MODEL_CIRCLE);

        scene_popMatrix();
    }

    scene_popMatrix();
    debug_popRenderScope();
}

static void drawControlPolygon(vec2 *ctrl, int n) {
    scene_pushMatrix();

    model_updateCurve(ctrl, NULL, n);
    shader_setColor(VEC3(0,1,1));
    model_drawCurve(n, 2.0f);

    scene_popMatrix();
}

static void drawCurve(InputData *data, vec2 *ctrl, float step, float width, int n) {
    scene_pushMatrix();

    if (data->curve.resolutionChanged || data->curve.buttonsChanged) {
        curve.numVertices = 0;
        for (float T = 0.0f; T <= 1.0f && curve.numVertices < CURVE_MAX_VERTICES; T += step) {
            data->curve.curveEval(ctrl, n, T, curve.vertices[curve.numVertices], &data->curve.buttonsChanged);
            curve.numVertices++;
        }

        // always interpolate last step
        data->curve.curveEval(ctrl, n, 1.0f, curve.vertices[curve.numVertices - 1], &data->curve.buttonsChanged);

        utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);

        data->curve.resolutionChanged = false;
    }

    model_updateCurve(curve.vertices, curve.normalVertices, curve.numVertices);
    shader_setColor(VEC3(1, 0, 0));
    model_drawCurve(curve.numVertices, width);

    scene_popMatrix();
}

static void drawConvexHull(int btnCnt) {
    vec2 points[BUTTON_COUNT];
    for (int i = 0; i < btnCnt; ++i) {
        glm_vec2_copy(buttons[i]->center, points[i]);
    }

    vec2 hull[BUTTON_COUNT + 1];
    int hullCount = utils_convexHullVec2(points, hull, btnCnt);

    model_updateCurve(hull, NULL, hullCount);
    shader_setColor(VEC3(0,1,0));
    model_drawCurve(hullCount, 2.0f);
}

static void drawClouds(InputData *data) {
    scene_pushMatrix();

    vec2 pos;
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
            1.0f,   // Center
            0.85f,  // Left
            0.85f,  // Right
            0.7f,   // Bottom left
            0.7f,   // Bottom right
            0.75f,  // Top
            0.6f,   // Far left
            0.6f,   // Far right
        };

        // Cloud parts from back to front for proper layering
        for (int j = 7; j >= 0; j--) {
            scene_pushMatrix();

            float baseSize = data->game.clouds.colliderRadius * 0.9f;
            float scale = baseSize * sizes[j];

            scene_translate(pos[0] + offsets[j][0], pos[1] + offsets[j][1], 0.0f);
            scene_scale(scale, scale * 0.85f, 1.0f); // Slightly flattened

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

static void drawAirplane(InputData *data) {
    scene_pushMatrix();

    // 1. Shadow (drawn first so it's behind)
    scene_pushMatrix();
    shader_setColor((vec3){0.3f, 0.3f, 0.4f}); // Dark shadow
    scene_translate(data->game.airplane.position[0] + 0.015f,
                   data->game.airplane.position[1] - 0.015f, 0.0f);
    scene_rotate(data->game.airplane.rotation * 180.0f / (float)M_PI, 0, 0, 1);
    scene_scale(0.16f, 0.21f, 1.0f);
    model_draw(MODEL_TRIANGLE);  // Still using triangle!
    scene_popMatrix();

    // 2. Main white body
    scene_pushMatrix();
    shader_setColor((vec3){0.95f, 0.95f, 1.0f}); // Paper white with slight blue tint
    scene_translate(data->game.airplane.position[0], data->game.airplane.position[1], 0.0f);
    scene_rotate(data->game.airplane.rotation * 180.0f / (float)M_PI, 0, 0, 1);
    scene_scale(0.15f, 0.2f, 1.0f);
    model_draw(MODEL_TRIANGLE);

    // 3. Center fold (darker triangle on top for paper fold effect)
    scene_pushMatrix();
    scene_scale(0.3f, 0.95f, 1.0f);  // Thinner triangle in the center
    shader_setColor((vec3){0.85f, 0.85f, 0.9f}); // Slightly darker for fold
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

static void initCurve(void) {
    InputData *input = getInputData();
    int btnCnt = input->curve.buttonCount;
    vec2 ctrl[BUTTON_COUNT];
    for (int k = 0; k < btnCnt; ++k) {
        ctrl[k][0] = buttons[k]->center[0];
        ctrl[k][1] = buttons[k]->center[1];
    }

    for (float T = 0.0f; T <= 1.0f && curve.numVertices < CURVE_MAX_VERTICES; T += input->curve.resolution) {
        input->curve.curveEval(ctrl, btnCnt, T, curve.vertices[curve.numVertices], &input->curve.buttonsChanged);
        curve.numVertices++;
    }

    input->curve.curveEval(ctrl, btnCnt, 1.0f, curve.vertices[curve.numVertices - 1], &input->curve.buttonsChanged);
    utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);
}

// PUBLIC

void initButtons(int btnCnt) { 
    for (int i = 0; i < btnCnt; i++) { 
        buttons[i] = &buttonStorage[i]; 
        buttons[i]->r = BUTTON_RADIUS; 
        if (i == 0) { 
            buttons[i]->center[0] = rd.right * START_BUTTON_EDGE_DIST; 
            buttons[i]->center[1] = 0.0f; 
        } else if (i == btnCnt - 1) { 
            buttons[i]->center[0] = rd.left * END_BUTTON_EDGE_DIST; 
            buttons[i]->center[1] = 0.0f; 
        } else { 
            float t = (float)i / (btnCnt- 1); 
            buttons[i]->center[0] = rd.right * START_BUTTON_EDGE_DIST * (1.0f - t) 
                                  + rd.left * END_BUTTON_EDGE_DIST * t; 
            buttons[i]->center[1] = 0.0f; 
        } 
    } 
    buttonInitilized = true;
}

// Funktion zum Zeichnen des Hintergrunds
static void drawGradientBackground(void) {
    // Depth-Test temporär deaktivieren für Hintergrund
    glDisable(GL_DEPTH_TEST);

    // Gradient Shader aktivieren
    shader_renderGradient();

    // Fullscreen Quad zeichnen
    glBindVertexArray(backgroundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Depth-Test wieder aktivieren falls nötig
    // glEnable(GL_DEPTH_TEST); // In 2D nicht nötig
}

void rendering_init(void) {
    memset(&rd, 0, sizeof(RenderingData));

    scene_lookAt((vec3) {0, 0, 1}, GLM_VEC3_ZERO, GLM_YUP);

    // Gradient Start
    float backgroundVertices[] = {
        // Position (NDC)
        -1.0f, -1.0f, 0.0f,  // Links unten
         1.0f, -1.0f, 0.0f,  // Rechts unten
        -1.0f,  1.0f, 0.0f,  // Links oben

         1.0f, -1.0f, 0.0f,  // Rechts unten
         1.0f,  1.0f, 0.0f,  // Rechts oben
        -1.0f,  1.0f, 0.0f   // Links oben
    };

    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);

    glBindVertexArray(backgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
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
        ctrl[k][0] = buttons[k]->center[0];
        ctrl[k][1] = buttons[k]->center[1];
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
    if (backgroundVBO) {
        glDeleteBuffers(1, &backgroundVBO);
    }
    if (backgroundVAO) {
        glDeleteVertexArrays(1, &backgroundVAO);
    }

    shader_cleanup();
}
   
static void updateEdgeButtons(int btnCnt) {
    if (buttons == NULL || buttons[0] == NULL) {
        return;
    }

    // First button -> right edge
    buttons[0]->center[0] = rd.right * START_BUTTON_EDGE_DIST;
    buttons[0]->center[1] = 0.0f;

    // Last button -> left edge
    buttons[btnCnt-1]->center[0] = rd.left * END_BUTTON_EDGE_DIST;
    buttons[btnCnt-1]->center[1] = 0.0f;
}

void rendering_resize(int width, int height, int btnCnt) {
    rd.screenRes[0] = width;
    rd.screenRes[1] = height;

    rd.aspect = (float) width / height;

    rd.left   = -BOUNDS * (rd.aspect >= 1 ? rd.aspect : 1);
    rd.right  =  BOUNDS * (rd.aspect >= 1 ? rd.aspect : 1);
    rd.bottom = -BOUNDS / (rd.aspect < 1  ? rd.aspect : 1);
    rd.top    =  BOUNDS / (rd.aspect < 1  ? rd.aspect : 1);

    scene_ortho(
        rd.left, rd.right, rd.bottom, rd.top,
        NEAR_PLANE, FAR_PLANE
    );

    if (buttonInitilized) {
        updateEdgeButtons(btnCnt);
    } else {
        initButtons(btnCnt);
    }
}
