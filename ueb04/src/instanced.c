#include "instanced.h"
#include <fhwcg/fhwcg.h>
#include "input.h"

struct CGMesh {
    GLuint vao, vbo, ebo;
    GLsizei numVertices, numIndices;
    GLenum mode; 
};

typedef struct {
    vec3 pos;
    vec3 acceleration;
    vec3 up;
    vec3 forward;
} ParticleInstance;

////////////////////////    LOCAL    ////////////////////////////

static struct {
    GLuint buffer;
    ParticleInstance *instances;
    int size;
} g_vbo = {
    .buffer = 0,
    .size = 0,
    .instances = NULL
};

////////////////////////    PUBLIC    ////////////////////////////

CGMesh* instanced_createMesh(
    const Vertex *vertices, const int numVerts, 
    const GLuint* indices, const int numInd, 
    GLenum mode
) {
    CGMesh *m = malloc(sizeof(CGMesh));

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVerts, vertices, GL_DYNAMIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Tex Coords
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
        m->vbo = 0;
    }

    if (m->ebo) { 
        glDeleteBuffers(1, &(m->ebo));
        m->ebo = 0;
    }

    if (m->vao) {
        glDeleteVertexArrays(1, &(m->vao));
        m->vao = 0;
    }

    free(m);
}

void instanced_draw(CGMesh *m, bool instanced) {
    glBindVertexArray(m->vao);

    if (m->numIndices) {
        if (instanced) {
            glDrawElementsInstanced(m->mode, m->numIndices, GL_UNSIGNED_INT, 0, g_vbo.size);
        } else {
            glDrawElements(m->mode, m->numIndices, GL_UNSIGNED_INT, 0);
        }
    } else {
        if (instanced) {
            glDrawArraysInstanced(m->mode, 0, m->numVertices, g_vbo.size);
        } else {
            glDrawArrays(m->mode, 0, m->numVertices);
        }
    }

    glBindVertexArray(0);
}

void instanced_init(void) {
    g_vbo.size = START_NUM_PARTICLES;
    g_vbo.instances = malloc(g_vbo.size * sizeof(ParticleInstance));
    glGenBuffers(1, &g_vbo.buffer);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo.buffer);
    glBufferData(GL_ARRAY_BUFFER, g_vbo.size * sizeof(ParticleInstance), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void instanced_bindAttrib(CGMesh *m) {
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo.buffer);

    // Offset Position
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, pos)
    );
    glVertexAttribDivisor(3, 1);

    // Acceleration
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, acceleration)
    );
    glVertexAttribDivisor(4, 1);

    // Up
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(
        5, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, up)
    );
    glVertexAttribDivisor(5, 1);

    // Forward
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(
        6, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (void*)offsetof(ParticleInstance, forward)
    );
    glVertexAttribDivisor(6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void instanced_resize(int count) {
    if (g_vbo.size == count) {
        return;
    }

    g_vbo.size = count;
    ParticleInstance *tmp = realloc(g_vbo.instances, g_vbo.size * sizeof(ParticleInstance));
    if (!tmp) {
        printf("Could not allocate memory in instanced_resize!");
        return;
    }

    g_vbo.instances = tmp;
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo.buffer);
    glBufferData(GL_ARRAY_BUFFER, g_vbo.size * sizeof(ParticleInstance), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void instanced_update(int count, vec3* pos, vec3* acceleration, vec3* up, vec3* forward) {
    instanced_resize(count);

    for (int i = 0; i < count; ++i) {
        glm_vec3_copy(pos[i], g_vbo.instances[i].pos);
        glm_vec3_copy(acceleration[i], g_vbo.instances[i].acceleration);
        glm_vec3_copy(up[i], g_vbo.instances[i].up);
        glm_vec3_copy(forward[i], g_vbo.instances[i].forward);
    }

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo.buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, g_vbo.size * sizeof(ParticleInstance), g_vbo.instances);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void instanced_cleanup(void) {
    glDeleteBuffers(1, &g_vbo.buffer);
    g_vbo.buffer = 0;
    g_vbo.size = 0;
    free(g_vbo.instances);
    g_vbo.instances = NULL;
}
