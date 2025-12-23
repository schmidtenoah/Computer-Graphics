#version 330 core

out vec4 fragColor;

uniform vec3 u_color;

flat in int vertexID;

void main() {
    vec3 color = ((vertexID % 2) == 0) ? u_color : vec3(1.0) - u_color;
    fragColor = vec4(color, 1.0);
}