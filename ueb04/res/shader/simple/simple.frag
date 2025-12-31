#version 430 core

out vec4 fragColor;

uniform vec3 u_color;
uniform bool u_drawInstanced;

flat in int vertexID;
flat in int instanceID;
flat in int isLeader;

void main() {
    vec3 color;

    if (u_drawInstanced && isLeader == 0) {
        // Leader ist rot
        color = vec3(1.0, 0.0, 0.0);
    } else if (u_drawInstanced) {
        // Normale Partikel: alterniere Farbe basierend auf Instanz, nicht Vertex
        color = ((instanceID % 2) == 0) ? u_color : vec3(1.0) - u_color;
    } else {
        // Nicht-instanziert: verwende Vertex-basierte Farbe
        color = ((vertexID % 2) == 0) ? u_color : vec3(1.0) - u_color;
    }

    fragColor = vec4(color, 1.0);
}