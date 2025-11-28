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
#include "physics.h"

/** Projection data*/
#define NEAR_PLANE 0.0001f
#define FAR_PLANE 200.0f
#define FOV_Y 45

/** Colors*/
#define SELECTED_COLOR VEC3(1, 0, 0)

/**
 * Rendering viewport and projection data.
 * Contains screen resolution and projection bounds.
 */
typedef struct {
    ivec2 screenRes;
    float aspect;
    float left, right, top, bottom;
} RenderingData;

////////////////////////    LOCAL    ////////////////////////////

/** Global rendering data (viewport, projection bounds, screen resolution) */
static RenderingData g_renderingData;

/**
 * Sets the View-Matrix based on the current active camera.
 */
static void updateCamera(InputData *data) {
    if (!data->cam.isFlying) {
        camera_getPosition(data->cam.data, data->cam.pos);
        camera_getFront(data->cam.data, data->cam.dir);
    }

    scene_look(data->cam.pos, data->cam.dir, GLM_YUP);
    shader_setCamPos(data->cam.pos);
}

/**
 * Updates the point light position and sends all relevant attributes to the shader.
 * @param data The InputData.
 */
static void updatePointLight(InputData *data) {
    utils_rotateAroundYAxis(
        &data->pointLight.posWS,
        &data->pointLight.currAngle,
        data->pointLight.center,
        data->pointLight.rotationRadius,
        data->pointLight.speed,
        data->deltaTime
    );

    shader_setPointLight(
        data->pointLight.color,
        data->pointLight.posWS,
        data->pointLight.falloff,
        data->pointLight.enabled,
        data->pointLight.ambientFactor
    );

    if (data->pointLight.visualize) {
        scene_pushMatrix();
        {
        scene_translateV(data->pointLight.posWS);
        scene_scale(0.1f, 0.1f, 0.1f);
        shader_setColor(data->pointLight.color);
        shader_setSimpleMVP();
        model_drawSimple(MODEL_SPHERE);
        }
        scene_popMatrix();
    }
}

static void drawCamFlightPath(InputData *data) {
    const int SEG = 128;
    vec3 p;
    for (int i = 0; i <= SEG; ++i) {
        float t = (float)i / (float)SEG;
        utils_evalBezier3D(
            data->cam.flight.p0,
            data->cam.flight.p1,
            data->cam.flight.p2,
            data->cam.flight.p3,
            t, p
        );

        scene_pushMatrix();
        {
            scene_translateV(p);
            scene_scaleV(VEC3X(0.003f));
            shader_setColor(VEC3(1, 1, 0));
            model_drawSimple(MODEL_SPHERE);
        }
        scene_popMatrix();
    }
}

static void drawControlPoints(InputData *data) {
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

static void drawSurface(InputData *data) {
    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);
    updatePointLight(data);
    scene_getMV(modelviewMat);
        
    // Set texture if enabled
    if (data->surface.useTexture) {
        GLuint texId = model_getTextureId(data->surface.currentTextureIndex);
        shader_setTexture(texId, true);
    } else {
        shader_setTexture(0, false);
    }
    
    model_drawSurface(data->showNormals, &viewMat, &modelviewMat);
}

static void drawObstacles(InputData *data) {
    scene_pushMatrix();

    bool showNormals = data->showNormals;
    mat4 modelviewMat, viewMat;
    scene_getMV(viewMat);

    for (int i = 0; i < OBSTACLE_COUNT; ++i) {
        scene_pushMatrix();

        Obstacle *o = &data->game.obstacles[i];
        scene_translateV(o->center);

        if (o->isParallel) {
            scene_rotate(90, 0, 1, 0);
        }

        scene_scale(o->length, 0.05f, o->width);
        scene_getMV(modelviewMat);

        model_draw(MODEL_CUBE, showNormals, true, &viewMat, &modelviewMat);

        scene_popMatrix();
    }

    scene_popMatrix();
}

////////////////////////    LOCAL    ////////////////////////////

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

    if (data->surface.showControlPoints) {
        drawControlPoints(data);
    }

    if (data->surface.showSurface) {
        drawSurface(data);
    }

    if (data->cam.flight.showPath) {
        drawCamFlightPath(data);
    }

    if (data->game.showObstacles) {
        drawObstacles(data);
    }
    
    physics_drawBalls();

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
