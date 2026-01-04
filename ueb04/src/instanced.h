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

/** mesh struct */
typedef struct CGMesh CGMesh;

/**
 * Vertex struct for mesh data.
 */
typedef struct {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texCoords[2];
} CGVertex;

/**
 * Creates a mesh from vertex and index data.
 * @param vertices Array of vertex data.
 * @param numVerts Number of vertices.
 * @param indices Array of indices (NULL for non-indexed).
 * @param numInd Number of indices (0 for non-indexed).
 * @param mode OpenGL primitive mode (GL_TRIANGLES, GL_LINES, ...).
 * @return Pointer to created mesh.
 */
CGMesh* instanced_createMesh(
    const CGVertex *vertices, const int numVerts, 
    const GLuint* indices, const int numInd, 
    GLenum mode
);

/**
 * Frees a mesh and its GPU resources.
 * @param m Pointer to mesh to dispose.
 */
void instanced_disposeMesh(CGMesh  *m);

/**
 * Draws a mesh, optionally using instanced rendering.
 * @param m Mesh to draw.
 * @param instanced If true, uses instanced rendering with current instance buffer.
 */
void instanced_draw(CGMesh *m, bool instanced);

/**
 * Draws a mesh with particle visualization shader.
 * Always uses instanced rendering.
 * @param m Mesh to draw.
 */
void instanced_drawParticleVis(CGMesh *m);

/**
 * Initializes the instanced rendering system.
 * Allocates the instance buffer with default particle count.
 */
void instanced_init(void);

/**
 * Cleans up instanced rendering resources.
 */
void instanced_cleanup(void);

/**
 * Binds instance attributes to a mesh.
 * Sets up vertex attribute pointers for instanced data.
 * @param m Mesh to bind attributes to.
 */
void instanced_bindAttrib(CGMesh *m);

/**
 * Resizes the instance buffer to accommodate a new particle count.
 * @param count New number of instances.
 */
void instanced_resize(int count);

/**
 * Updates instance buffer with new particle data.
 * @param count Number of particles.
 * @param pos Array of particle positions.
 * @param acceleration Array of particle accelerations.
 * @param up Array of particle up vectors.
 * @param forward Array of particle forward vectors.
 */
void instanced_update(int count, vec3* pos, vec3* acceleration, vec3* up, vec3* forward);

#endif // INSTANCED_H
