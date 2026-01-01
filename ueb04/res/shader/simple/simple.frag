#version 430 core

out vec4 fragColor;

uniform vec3 u_color;
uniform bool u_drawInstanced;

flat in int isLeader;
in vec3 vColor;

void main() {
    vec3 color;

    if (u_drawInstanced && isLeader == 0) {
        color = vec3(1.0, 0.0, 0.0);
    } else {
        color = vColor;
    }

    fragColor = vec4(color, 1.0);
}