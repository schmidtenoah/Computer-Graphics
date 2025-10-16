#version 430 core

/**
 * @file
 * 
 * @brief Ein Shader zum Anzeigen von Normalen von 3D Modellen aus Dreiecken.
 *
 * Siehe Normals Demo für ein Beispiel.
 *
 */

#ifndef _DOC
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 pos;
    vec3 normal;
} geo_in[];
#endif

/** aktuelle Projektionsmatrix */
uniform mat4 u_projMatrix;

/** Länge der Linien */
uniform float u_normalLength = 1;

void main() {
    for (int i = 0; i < 3; i++)
    {
        // Startpunkt in den Clipping Space transformieren
        gl_Position = u_projMatrix * vec4(geo_in[i].pos, 1.0);
        EmitVertex(); // Startpunkt erzeugen

        // Endpunkt der Linie berechnen
        vec3 endPos = geo_in[i].pos + normalize(geo_in[i].normal) * u_normalLength;

        // Endpunkt in den Clipping Space transformieren
        gl_Position = u_projMatrix * vec4(endPos, 1.0);
        EmitVertex(); // Endpunkt erzeugen

        EndPrimitive(); // Linie beenden
    }
}
