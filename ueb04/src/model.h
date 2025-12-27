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
    MODEL_TRIANGLE,
    MODEL_LINE,
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
 * Draws a model with textured shader
 * @param model Model type to draw
 */
void model_drawTextured(ModelType model, bool texOrder1);

/**
 * Draws a model with simple color shader
 * @param model Model type to draw
 */
void model_drawSimple(ModelType model);

void model_drawInstanced(ModelType model);

#endif // MODEL_H