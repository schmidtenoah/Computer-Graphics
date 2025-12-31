#version 430 core

#include "../utils.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoords;

// Instance
layout(location = 3) in vec3 offset;
layout(location = 4) in vec3 acceleration;
layout(location = 5) in vec3 up;
layout(location = 6) in vec3 forward;

uniform mat4 u_mvpMatrix;
uniform bool u_drawInstanced;
uniform vec3 u_localScale;
uniform int u_leaderIdx;

flat out int vertexID;
flat out int instanceID;
flat out int isLeader;

void main() {
    vec3 worldPos = pos;
    vec3 upVec = up;

    if (u_drawInstanced) {
        // Verwende die berechnete Basis für die Rotation
        transform(worldPos, forward, upVec, u_localScale, offset);
        isLeader = ((u_leaderIdx != -1) && (gl_InstanceID == u_leaderIdx)) ? 0 : 1;
        instanceID = gl_InstanceID;
    }

    gl_Position = u_mvpMatrix * vec4(worldPos, 1.0);
    vertexID = gl_VertexID;
}