/**
 * @file
 * 
 * @brief
 * Modul für die Verwaltung von Meshes.
 * 
 * @copyright 2024, FH Wedel
 * Autor: Philipp Munz
 */

#ifndef FHWCG_MESH_H
#define FHWCG_MESH_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

struct mesh;

/** Datenstruktur, die ein Mesh repräsentiert */
typedef struct mesh Mesh;

/** Datenstruktur, die einen Vertex repräsentiert */
typedef struct {
    /** Position des Vertex */
    GLfloat position[3];
    /** Normale des Vertex */
    GLfloat normal[3];
    /** Texturkoordinate des Vertex */
    GLfloat texCoords[2];
} Vertex;

/** Erzeugt einen Vertex mit Position, Normale und Texturkoordinate */
#define Vertex3Tex(x, y, z, nx, ny, nz, s, t) (Vertex){.position={x, y, z}, .normal={nx, ny, nz}, .texCoords={s, t}}

/** Erzeugt einen Vertex mit Position und Normale */
#define Vertex3(x, y, z, nx, ny, nz) Vertex3Tex(x, y, z, nx, ny, nz, 0, 0)

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt ein Kugel Mesh inklusive der Normalen.  
 * Der Nordpol zeigt dabei in die positive Z-Achse.
 * 
 * @param[in] numSlices Anzahl an horizontalen Unterteilungen
 * @param[in] numStacks Anzahl an vertikalen Unterteilungen
 * 
 * @return Adresse des Kugelmeshes
 */
Mesh* mesh_createSphere(int numSlices, int numStacks);

/**
 * Zeichnet ein Mesh.
 * 
 * Locations:  
 * 0: vec3 Position  
 * 1: vec3 Normale  
 * 2: vec2 Texturkoordinate
 * 
 * @param[in] m Adresse des Meshes
 */
void mesh_drawMesh(Mesh* m);

/**
 * Zeichnet ein Mesh mithilfe eines Indexarrays.
 * 
 * @note Es sollte ein Mesh eigentlich immer direkt mit seinen Indices erstellt
 * (siehe mesh_createMesh()) und dann per mesh_drawMesh() gezeichnet werden.  
 * Diese Funktion darf nur genutzt werden, wenn die Indices dynamisch zur Laufzeit geändert werden.
 * 
 * @param[in] m Adresse des Meshes
 * @param[in] indices die Indices dieses Rendervorgangs
 * @param[in] numIndices Anzahl an Indices
 */
void mesh_drawMeshElements(Mesh* m, const GLuint* const indices, const int numIndices);

/**
 * Gibt den Speicher eines Meshes wieder frei.
 * 
 * @param[in,out] mesh Adresse der Mesh-Variablen. Die referenzierte Variable wird auf NULL gesetzt.
 */
void mesh_disposeMesh(Mesh** mesh);

/**
 * Erzeugt ein Mesh aus Vertices und (optional) Indices.
 * 
 * @pre vertices != NULL && numVertices > 0
 * @pre (indices == NULL && numIndices == 0) || (indices != NULL && numIndices > 0)
 * 
 * @param[in] name Name des Meshes in RenderDoc
 * @param[in] vertices Vertices aus denen das Mesh besteht
 * @param[in] numVertices Anzahl an Vertices
 * @param[in] indices Indices aus denen die Primitive gebaut werden oder NULL
 * @param[in] numIndices Anzahl der Indices. Muss 0 sein wenn keine Indices genutzt werden
 * @param[in] mode Typ der Primitive (GL_LINES, GL_TRIANGLES etc.)
 * 
 * @return Adresse des erzeugten Meshes
 */
Mesh* mesh_createMesh(const char* const name, const Vertex* const vertices, const int numVertices, const GLuint* const indices, const int numIndices, GLenum mode);

#endif // FHWCG_MESH_H