#version 430 core

out vec4 fragColor;

uniform bool u_drawInstanced;
flat in int isLeader;

const vec3 shadowColor = vec3(0.9, 0.9, 0.9);

void main() {
    vec3 color = shadowColor;

    if (u_drawInstanced && isLeader == 0) {
        color = vec3(1.0, 0.0, 0.0);
    }

    fragColor = vec4(color, 1.0);
}
