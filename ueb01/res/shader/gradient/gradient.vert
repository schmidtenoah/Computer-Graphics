#version 430 core

layout(location = 0) in vec3 pos;

out vec2 fragCoord;

void main() {
    gl_Position = vec4(pos, 1.0);
    fragCoord = pos.xy * 0.5 + 0.5; // Von [-1,1] zu [0,1]
}