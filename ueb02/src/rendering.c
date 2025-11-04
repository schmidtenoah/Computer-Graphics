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
#define NEAR_PLANE 0.0001f
#define FAR_PLANE 200.0f
#define FOV_Y 45

/** Colors*/
#define SELECTED_COLOR VEC3(1, 0, 0)

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


/**
 * Curve rendering data.
 */
static struct {
    vec2 vertices[CURVE_MAX_VERTICES];
    vec3 normalVertices[CURVE_MAX_VERTICES];
    int numVertices;
} curve;

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

        //utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);

        data->curve.resolutionChanged = false;
    }

    model_updateCurve(curve.vertices, curve.normalVertices, curve.numVertices);
    if (data->curve.buttonsChanged) data->curve.buttonsChanged = false;
    shader_setColor(VEC3(1, 0, 0));
    model_drawCurve(curve.numVertices, width);

    scene_popMatrix();
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
    vec2 ctrl[7];
    for (int k = 0; k < btnCnt; ++k) {
        //ctrl[k][0] = g_buttons[k]->center[0];
        //ctrl[k][1] = g_buttons[k]->center[1];
    }

    for (float T = 0.0f; T <= 1.0f && curve.numVertices < CURVE_MAX_VERTICES; T += input->curve.resolution) {
        input->curve.curveEval(ctrl, btnCnt, T, curve.vertices[curve.numVertices], &input->curve.buttonsChanged);
        curve.numVertices++;
    }

    input->curve.curveEval(ctrl, btnCnt, 1.0f, curve.vertices[curve.numVertices - 1], &input->curve.buttonsChanged);
    //utils_calcNormals(curve.vertices, curve.normalVertices, curve.numVertices);
}

static void updateCamera(InputData *data) {
    if (!data->cam.isFlying) {
        camera_getPosition(data->cam.data, data->cam.pos);
        camera_getFront(data->cam.data, data->cam.dir);
    }

    scene_look(data->cam.pos, data->cam.dir, GLM_YUP);
}

////////////////////////    LOCAL    ////////////////////////////

/*void initButtons(int btnCnt) { 
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
}*/

void rendering_init(void) {
    memset(&g_renderingData, 0, sizeof(RenderingData));

    // OpenGL Flags
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    shader_load();
}

void rendering_draw(void) {
    InputData* data = getInputData();

    if(data->showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
    glEnable(GL_DEPTH_TEST);

    debug_pushRenderScope("Scene");
    scene_pushMatrix();

    updateCamera(data);
    mat4 viewMat, modelviewMat;
    scene_getMV(viewMat);

    if (data->surface.showControlPoints) {
        for (int i = 0; i < data->surface.controlPoints.size; ++i) {
        scene_pushMatrix();

        bool isSelected = data->selection.selectedCp == i;

        scene_translateV(data->surface.controlPoints.data[i]);
        scene_scaleV(isSelected ? VEC3X(0.1f) : VEC3X(0.01f));
        vec3 *idxColor = (data->selection.selectedCp == i) ? 
            &SELECTED_COLOR : &VEC3X(i / data->surface.controlPoints.size)
        ;

        shader_setColor(*idxColor);
        model_drawSimple(MODEL_SPHERE);

        scene_popMatrix();
        }
    }

    if (data->surface.showSurface) {
        scene_getMV(modelviewMat);
        model_drawSurface(data->showNormals, &viewMat, &modelviewMat);
    }

    scene_popMatrix();
    debug_popRenderScope();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void rendering_cleanup(void) {
    shader_cleanup();
    camera_deleteCamera(&getInputData()->cam.data);
}

void rendering_resize(int width, int height) {
    g_renderingData.screenRes[0] = width;
    g_renderingData.screenRes[1] = height;

    g_renderingData.aspect = (float) width / height;

    scene_perspective(FOV_Y, g_renderingData.aspect, NEAR_PLANE, FAR_PLANE);
}
