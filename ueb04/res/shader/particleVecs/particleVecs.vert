#version 430 core

#include "../utils.glsl"

layout(location = 0) in vec3 pos;

// Instance
layout(location = 3) in vec3 offset;
layout(location = 4) in vec3 acceleration;
layout(location = 5) in vec3 up;
layout(location = 6) in vec3 forward;

uniform mat4 u_mvpMatrix;
uniform vec3 u_localScale;

out VS_OUT {
    vec3 worldPos;
    vec3 acceleration;
    vec3 up;
} vs_out;

void main() {
    vec3 upVec = up;
    vec3 worldPos = pos;
    transform(worldPos, forward, upVec, u_localScale, offset);
    
    vs_out.worldPos = worldPos;
    vs_out.acceleration = acceleration;
    vs_out.up = upVec;

    gl_Position = u_mvpMatrix * vec4(vs_out.worldPos, 1.0);
}
