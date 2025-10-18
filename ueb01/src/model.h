#ifndef MODEL_H
#define MODEL_H

#include <fhwcg/fhwcg.h>

typedef enum {
    // stored as Mesh
    MODEL_SQUARE,
    MODEL_CIRCLE,
    MODEL_TRIANGLE,
    MODEL_STAR,
    MODEL_MESH_COUNT,

    // stored with VAO/VBO
    MODEL_CURVE
} ModelType;

void model_init(void);

void model_cleanup(void);

void model_draw(ModelType model);

void model_drawCurve(int numVertices, float lineWidth);

void model_updateCurve(vec2 *vertices, vec3 *normalVertices, int numVertices);

#endif // MODEL_H