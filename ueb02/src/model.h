/**
 * @file model.h
 * @brief Model geometry definitions and drawing functions.
 *
 * Defines model types (circle, square, star, triangle, curve) and provides
 * functions to initialize, draw and update models.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef MODEL_H
#define MODEL_H

#include <fhwcg/fhwcg.h>

/**
 * Enum of model types.
 *
 * Models SQUARE, CIRCLE, TRIANGLE, STAR, MODEL_MESH_COUNT
 * are stored as static Mesh objects.
 *
 * MODEL_CURVE is stored with VAO/VBO for real-time updates.
 */
typedef enum {
    // stored as Mesh
    MODEL_SPHERE,
    MODEL_MESH_COUNT,

    // stored with VAO/VBO
    MODEL_CURVE,
    MODEL_SURFACE
} ModelType;

/**
 * Initializes all models. Creates  meshes (circle, square, star, triangle)
 * and VAO/VBO for curve model.
 */
void model_init(void);

/**
 * Cleans up all model resources.
 * Disposes meshes and deletes curve VAO/VBO.
 */
void model_cleanup(void);

/**
 * Draws model (circle, square, star or triangle)
 * Sets the MVP and renders the given model mesh.
 *
 * @param model The model type to draw
 * @note @param model must be < MODEL_MESH_COUNT
 * @param drawNormals If Normals should be drawn
 */
void model_draw(ModelType model, bool drawNormals, mat4 *viewMat, mat4 *modelViewMat);

void model_drawSimple(ModelType model);

/**
 * Draws the curve as a line strip.
 * Also renders normal vectors if enabled in settings.
 *
 * @param numVertices Number of vertices in the curve
 * @param lineWidth Width of the curve line
 */
void model_drawCurve(int numVertices, float lineWidth);

void model_drawSurface(bool drawNormals, mat4 *viewMat, mat4 *modelviewMat);

void model_updateSurface(vec3 *vertices, vec3 *normals, vec2 *texcoords, int dim);

/**
 * Updates the curve vertex buffer with new positions and normals
 * for dynamic rendering.
 *
 * @param vertices 2D Vector of vertex positions
 * @param normalVertices 3D Vector of 3D normal vectors
 * @param numVertices Number of vertices to update
 */
void model_updateCurve(vec2 *vertices, vec3 *normalVertices, int numVertices);

#endif // MODEL_H