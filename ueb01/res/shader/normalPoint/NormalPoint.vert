#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

out VS_OUT {
    vec3 pos;
    vec3 normal;
} vs_out;

uniform mat4 u_modelViewMatrix;
uniform mat4 u_normalMatrix;

void main() {
    vs_out.pos = vec3(u_modelViewMatrix * vec4(pos, 1.0));
    vs_out.normal = normalize(vec3(u_normalMatrix * vec4(normal, 0.0)));
}
