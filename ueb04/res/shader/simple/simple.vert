#version 330 core

layout(location = 0) in vec3 a_position;

uniform mat4 u_mvpMatrix;
flat out int vertexID;

void main() {
    vertexID = gl_VertexID;
    gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
}