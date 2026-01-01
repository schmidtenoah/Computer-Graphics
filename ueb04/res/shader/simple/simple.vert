#version 430 core

#include "../utils.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoords;

// Instance
layout(location = 4) in vec3 offset;
layout(location = 5) in vec3 acceleration;
layout(location = 6) in vec3 up;
layout(location = 7) in vec3 forward;

uniform mat4 u_mvpMatrix;
uniform bool u_drawInstanced;
uniform vec3 u_localScale;
uniform int u_leaderIdx;
uniform vec3 u_color;

flat out int isLeader;
out vec3 vColor;

void main() {
    vec3 worldPos = pos;
    vec3 upVec = up;

    if (u_drawInstanced) {
        transform(worldPos, forward, upVec, u_localScale, offset);
        isLeader = ((u_leaderIdx != -1) && (gl_InstanceID == u_leaderIdx)) ? 0 : 1;
    }

    gl_Position = u_mvpMatrix * vec4(worldPos, 1.0);
    vColor = (gl_VertexID % 3 == 0) ? u_color : vec3(1.0) - u_color;
}