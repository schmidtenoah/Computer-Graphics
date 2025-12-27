#version 430 core

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
flat out int isLeader;

void main() {
    vec3 worldPos = pos;

    if (u_drawInstanced) {
        /*vec3 f = normalize(forward);
        vec3 u = vec3(0.0, 1.0, 0.0);
        if (abs(dot(f, u)) > 0.99) u = vec3(1.0, 0.0, 0.0);
        vec3 r = normalize(cross(u, f));
        u = cross(f, r);

        worldPos = offset + (r * pos.x + u * pos.y, f * pos.z);*/

        worldPos = pos * u_localScale + offset;
        isLeader = ((u_leaderIdx != -1) && (gl_InstanceID == u_leaderIdx)) ? 0 : 1;
    }

    gl_Position = u_mvpMatrix * vec4(worldPos, 1.0);
    vertexID = gl_VertexID;
}