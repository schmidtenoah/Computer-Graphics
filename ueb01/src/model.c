#include "model.h"
#include "shader.h"
#include "rendering.h"
#include "input.h"

#define CIRCLE_VERTEX_COUNT 64
#define STAR_VERTEX_COUNT 10

// LOCAL

static Mesh *models[MODEL_MESH_COUNT];
static GLuint curveVAO = 0, curveVBO = 0;

// Buffer to convert vec2 vertices in a flat floart array
static float buffer[CURVE_MAX_VERTICES * 3];

/**
 * Creates a unit square mesh in the xy-plane.
 * Side Length: 1.
 */
static void model_initSquare(void) {
    const Vertex quadVertices[] = {
        Vertex3Tex(-0.5f, -0.5f, 0.0f, 0, 0, 1, 0.0f, 0.0f),
        Vertex3Tex( 0.5f, -0.5f, 0.0f, 0, 0, 1, 1.0f, 0.0f),
        Vertex3Tex(-0.5f,  0.5f, 0.0f, 0, 0, 1, 0.0f, 1.0f),
        Vertex3Tex( 0.5f,  0.5f, 0.0f, 0, 0, 1, 1.0f, 1.0f),
    };

    const GLuint quadIndices[] = {
        0, 1, 2, 2, 1, 3
    };

    models[MODEL_SQUARE] = mesh_createMesh("Square", quadVertices, 4, quadIndices, 6, GL_TRIANGLES);
}

/**
 * Creates a unit circle mesh in the xy-plane.
 * Radius: 1.
 */
static void model_initCircle(void) {
    Vertex circleVertices[CIRCLE_VERTEX_COUNT + 2];
    circleVertices[0] = Vertex3Tex(0.0f, 0.0f, 0.0f, 0, 0, 1, 0.5f, 0.5f);

    for (int i = 1; i <= CIRCLE_VERTEX_COUNT; ++i) {
        const float angle = (float) (2 * M_PI * i) / CIRCLE_VERTEX_COUNT;
        float x = cosf(angle);
        float y = sinf(angle);
        circleVertices[i] = Vertex3Tex(x, y, 0.0f, 0, 0, 1, x + 0.5f, y + 0.5f);
    }

    circleVertices[CIRCLE_VERTEX_COUNT + 1] = circleVertices[1];
    models[MODEL_CIRCLE] = mesh_createMesh("Circle", circleVertices, CIRCLE_VERTEX_COUNT + 2, NULL, 0, GL_TRIANGLE_FAN);
}

/**
 * Creates a unit star mesh in the xy-plane.
 * Max. peak radius: 1, other 0.5.
 */
static void model_initStar(void) {
    Vertex starVertices[STAR_VERTEX_COUNT + 2];
    starVertices[0] = Vertex3Tex(0.0f, 0.0f, 0.0f, 0, 0, 1, 0.5f, 0.5f);

    const float step = (2.0f * (float)M_PI) / (float)STAR_VERTEX_COUNT;

    for (int i = 1; i <= STAR_VERTEX_COUNT; ++i) {
        float angle = (i - 1) * step;
        float radius = 0.5f * ((i % 2 == 0) ? 0.5f : 1.0f);
        starVertices[i] = Vertex3Tex(
            radius * cosf(angle),
            radius * sinf(angle),
            0.0f, 0, 0, 1, 0.5f, 0.5f
        );
    }

    // Close the loop
    starVertices[STAR_VERTEX_COUNT + 1] = starVertices[1];

    models[MODEL_STAR] = mesh_createMesh("Star", starVertices, STAR_VERTEX_COUNT + 2, NULL, 0, GL_TRIANGLE_FAN);
}

/**
 * Creates a unit triangle mesh in the xy-plane.
 * Peak in y-direction. 
 * Side Length: 1.
 */
static void model_initTriangle(void) {
    Vertex triangleVertices[3];
    triangleVertices[0] =  Vertex3Tex(0.0f, 1, 0.0f, 0, 0, 1, 0.5f, 0.5f);
    triangleVertices[1] =  Vertex3Tex(-0.5f, -0.5f, 0.0f, 0, 0, 1, 0.5f, 0.5f);
    triangleVertices[2] =  Vertex3Tex(0.5f, -0.5f, 0.0f, 0, 0, 1, 0.5f, 0.5f);
    
    models[MODEL_TRIANGLE] = mesh_createMesh("Triangle", triangleVertices, 3, NULL, 0, GL_TRIANGLES);
}

static void model_initCurve(void) {
    glGenVertexArrays(1, &curveVAO);
    glGenBuffers(1, &curveVBO);

    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);

    // dynamic draw and reservation for largest possible curve
    glBufferData(GL_ARRAY_BUFFER, CURVE_MAX_VERTICES * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));


    glBindVertexArray(0);
}


// PUBLIC

void model_init(void) {
    model_initCircle();
    model_initSquare();
    model_initStar();
    model_initTriangle();
    model_initCurve();
}

void model_cleanup(void) {
    for (int i = 0; i < MODEL_MESH_COUNT; ++i) {
        if (models[i] == NULL) {
            continue;
        }

        mesh_disposeMesh(&(models[i]));
        models[i] = NULL;
    }
    
    glDeleteBuffers(1, &curveVBO);
    glDeleteVertexArrays(1, &curveVAO);
}

void model_draw(ModelType model) {
    if (model >= MODEL_MESH_COUNT) {
        return;
    }

    shader_setMVP();
    mesh_drawMesh(models[model]);
}

void model_drawCurve(int numVertices, float lineWidth) {
    glBindVertexArray(curveVAO);

    shader_setMVP();
    glLineWidth(lineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, numVertices);

    if (getInputData()->curve.showNormals) {
        shader_setNormals();
        glDrawArrays(GL_POINTS, 0, numVertices);
    }

    glBindVertexArray(0);
}

void model_updateCurve(vec2 *vertices, vec3 *normalVertices, int numVertices) {
    Vertex curveData[CURVE_MAX_VERTICES];

    for (int i = 0; i < numVertices; ++i) {
        curveData[i].position[0] = vertices[i][0];
        curveData[i].position[1] = vertices[i][1];
        curveData[i].position[2] = 0.0f;

        curveData[i].normal[0] = (normalVertices == NULL) ? 0 : normalVertices[i][0];
        curveData[i].normal[1] = (normalVertices == NULL) ? 0 : normalVertices[i][1];
        curveData[i].normal[2] = (normalVertices == NULL) ? 0 : normalVertices[i][2];
    }

    glBindVertexArray(curveVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * sizeof(Vertex), curveData);
    glBindVertexArray(0);
}
