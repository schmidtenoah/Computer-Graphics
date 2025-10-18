#version 430 core

out vec4 FragColor;
uniform vec3 u_color = vec3(0, 0, 1);

void main() {
    FragColor = vec4(u_color, 1.0);
}
