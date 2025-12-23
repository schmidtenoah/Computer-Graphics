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

typedef struct {
    GLuint vao, vbo, ebo;
    GLsizei numVertices, numIndices;
    GLenum mode; 
} CGMesh;

CGMesh* instanced_createMesh(const Vertex *vertices, const int numVerts, const GLuint* const indices, const int numInd, GLenum mode);

void instanced_disposeMesh(CGMesh  *m);

void instanced_draw(CGMesh *m);

void instanced_init(void);

void instanced_cleanup(void);

void instanced_bindAttrib(CGMesh *m);

#endif // INSTANCED_H