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
 * Loads textures for surface rendering.
 * Loads 3 different textures for the surface.
 */
void model_loadTextures(void);

/**
 * Returns the current texture ID based on index.
 * 
 * @param index Texture index (0-2)
 * @return GLuint texture ID
 */
GLuint model_getTextureId(int index);

/**
 * Draws model (circle, square, star or triangle)
 * Sets the MVP and renders the given model mesh.
 *
 * @param model The model type to draw
 * @note @param model must be < MODEL_MESH_COUNT
 * @param drawNormals If Normals should be drawn
 */
void model_draw(ModelType model, bool drawNormals, bool useBallMat, mat4 *viewMat, mat4 *modelViewMat);

/**
 * Draws the given model via the Simple-Shader (only color).
 * @param model The type of the model to draw.
 */
void model_drawSimple(ModelType model);

/**
 * Draws the Surface via the Model-Shader.
 * @param drawNormals If the Normals of the surface should be drawn.
 * @param viewMat The View-Matrix for the Model-Shader.
 * @param modelviewMat The Model-View-Matrix for the Model-Shader.
 */
void model_drawSurface(bool drawNormals, mat4 *viewMat, mat4 *modelviewMat);

/**
 * Dynamically updates the surface mesh based on the given main vertices.
 * @param vertices The main vertices of the surface (no indice vertices).
 * @param normals The normals belonging to the vertices.
 * @param texcoords The texture coordinates belonging to the vertices.
 * @param dim The dimension of the 2D-Surface (#vertices == dim^2).
 */
void model_updateSurface(vec3 *vertices, vec3 *normals, vec2 *texcoords, int dim);

#endif // MODEL_H
