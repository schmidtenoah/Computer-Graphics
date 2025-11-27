#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

uniform mat4 u_mvpMatrix;

/**
 * Simple Vertex Shader, transforms the vertex with the mvp-matrix.
 */
void main(void) {
    gl_Position = u_mvpMatrix * vec4(pos, 1);
}