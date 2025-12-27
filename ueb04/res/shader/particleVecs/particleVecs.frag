#version 430 core

out vec4 fragColor;
in vec3 gsColor;

void main() {
    fragColor = vec4(gsColor, 1.0);
}
