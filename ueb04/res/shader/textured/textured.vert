#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 tex;

out vec2 texCoords;

uniform mat4 u_mvpMatrix;

void main() {
    texCoords = tex;
    gl_Position = u_mvpMatrix * vec4(pos, 1.0);
}