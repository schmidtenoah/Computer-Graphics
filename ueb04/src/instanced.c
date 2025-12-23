#include "instanced.h"
#include <fhwcg/fhwcg.h>

static GLuint instanceVBO = 0;

CGMesh* instanced_createMesh(const Vertex *vertices, const int numVerts, const GLuint* const indices, const int numInd, GLenum mode) {
    CGMesh *m = malloc(sizeof(CGMesh));

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVerts, vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    m->numVertices = numVerts;
    m->vao = vao;
    m->vbo = vbo;

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) *numInd, indices, GL_DYNAMIC_DRAW);
    m->mode = mode;
    m->numIndices = numInd;
    m->ebo = ebo;

    glBindVertexArray(0);

    return m;
}

void instanced_disposeMesh(CGMesh *m) {
    if (m->vbo) {
        glDeleteBuffers(1, &(m->vbo));
    }
    if (m->ebo) { 
        glDeleteBuffers(1, &(m->ebo));
    }
    if (m->vao) {
        glDeleteVertexArrays(1, &(m->vao));
    }
    free(m);
}


void instanced_draw(CGMesh *m) {
    NK_UNUSED(m);
    /*glBindVertexArray(m->vao);
    glDrawElementsInstanced(m->mode, m->numIndices, GL_UNSIGNED_INT, 0, COUNT);

    glBindVertexArray(0);*/
}

void instanced_init(void) {

    /*glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trans), trans, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);*/
}

void instanced_bindAttrib(CGMesh *m) {
    glBindVertexArray(m->vao);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void instanced_cleanup(void) {
    glDeleteBuffers(1, &instanceVBO);
    instanceVBO = 0;
}
