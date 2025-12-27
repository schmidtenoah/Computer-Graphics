#version 330 core

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
flat out int vertexID;

void main() {
    vec3 worldPos = pos;

    if (u_drawInstanced) {
        vec3 f = normalize(forward);
        vec3 u = normalize(up);
        vec3 r = normalize(cross(f, u));

        mat3 basis = mat3(r, u, f);
        worldPos = offset + basis * pos;
    }

    gl_Position = u_mvpMatrix * vec4(worldPos, 1.0);
    vertexID = gl_VertexID;
}