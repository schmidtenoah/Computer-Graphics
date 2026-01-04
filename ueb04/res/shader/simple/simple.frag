#version 430 core

out vec4 fragColor;

uniform vec3 u_color;
uniform bool u_drawInstanced;
uniform bool u_hardColor;

in vec3 vBary;
flat in int isLeader;
in vec3 vColor;

void main() {
    vec3 color;

    if (u_drawInstanced && isLeader == 0) {
        color = vec3(1.0, 0.0, 0.0);
    } else {
        if (u_hardColor) {
            float t = smoothstep(-0.2, 0.2, vBary.x - vBary.y);
            color = mix(u_color, vec3(1.0) - u_color, t);
        } else {
            color = vColor;
        }
    }

    fragColor = vec4(color, 1.0);
}