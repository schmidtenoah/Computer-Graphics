/**
* @file logic.h
 * @brief Minimal logic interface
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef INSTANCED_H
#define INSTANCED_H

#include <fhwcg/fhwcg.h>
#include "input.h"

typedef struct CGMesh CGMesh;

typedef struct {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texCoords[2];
    GLint id;
} CGVertex;

CGMesh* instanced_createMesh(
    const CGVertex *vertices, const int numVerts, 
    const GLuint* indices, const int numInd, 
    GLenum mode
);

void instanced_disposeMesh(CGMesh  *m);

void instanced_draw(CGMesh *m, bool instanced);

void instanced_drawParticleVis(CGMesh *m);

void instanced_init(void);

void instanced_cleanup(void);

void instanced_bindAttrib(CGMesh *m);

void instanced_resize(int count);

void instanced_update(int count, vec3* pos, vec3* acceleration, vec3* up, vec3* forward);

#endif // INSTANCED_H
