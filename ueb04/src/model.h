/**
* @file model.h
 * @brief Model geometry definitions
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef MODEL_H
#define MODEL_H

#include <fhwcg/fhwcg.h>

/** Available model types */
typedef enum {
    MODEL_SPHERE,
    MODEL_CUBE,
    MODEL_MESH_COUNT
} ModelType;

/**
 * Initializes all models
 */
void model_init(void);

/**
 * Cleans up all model resources
 */
void model_cleanup(void);

/**
 * Loads textures for room
 */
void model_loadTextures(void);

/**
 * Returns texture ID for given index
 * @param index Texture index (0 = floor, 1/2 = walls/ceiling)
 * @return OpenGL texture ID
 */
GLuint model_getTextureId(int index);

/**
 * Draws a model with textured shader
 * @param model Model type to draw
 */
void model_drawTextured(ModelType model);

/**
 * Draws a model with simple color shader
 * @param model Model type to draw
 */
void model_drawSimple(ModelType model);

#endif // MODEL_H